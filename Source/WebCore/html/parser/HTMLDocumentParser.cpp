/*
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "html/parser/HTMLDocumentParser.h"

#include "html/parser/AtomicHTMLToken.h"
#include "html/parser/BackgroundHTMLParser.h"
#include "html/parser/CompactHTMLToken.h"
#include "page/ContentSecurityPolicy.h"
#include "dom/DocumentFragment.h"
#include "loader/DocumentLoader.h"
#include "dom/Element.h"
#include "page/Frame.h"
#include "html/parser/HTMLIdentifier.h"
#include "HTMLNames.h"
#include "html/parser/HTMLParserScheduler.h"
#include "html/parser/HTMLParserThread.h"
#include "html/parser/HTMLTokenizer.h"
#include "html/parser/HTMLPreloadScanner.h"
#include "html/parser/HTMLTreeBuilder.h"
#include "html/HTMLDocument.h"
#include "html/parser/NestingLevelIncrementer.h"
#include "page/Settings.h"
#include <wtf/Functional.h>

namespace WebCore {

using namespace HTMLNames;

// This is a direct transcription of step 4 from:
// http://www.whatwg.org/specs/web-apps/current-work/multipage/the-end.html#fragment-case
static HTMLTokenizer::State tokenizerStateForContextElement(Element* contextElement, bool reportErrors, const HTMLParserOptions& options)
{
    if (!contextElement)
        return HTMLTokenizer::DataState;

    const QualifiedName& contextTag = contextElement->tagQName();

    if (contextTag.matches(titleTag) || contextTag.matches(textareaTag))
        return HTMLTokenizer::RCDATAState;
    if (contextTag.matches(styleTag)
        || contextTag.matches(xmpTag)
        || contextTag.matches(iframeTag)
        || (contextTag.matches(noembedTag) && options.pluginsEnabled)
        || (contextTag.matches(noscriptTag) && options.scriptEnabled)
        || contextTag.matches(noframesTag))
        return reportErrors ? HTMLTokenizer::RAWTEXTState : HTMLTokenizer::PLAINTEXTState;
    if (contextTag.matches(plaintextTag))
        return HTMLTokenizer::PLAINTEXTState;
    return HTMLTokenizer::DataState;
}

HTMLDocumentParser::HTMLDocumentParser(HTMLDocument* document, bool reportErrors)
    : ScriptableDocumentParser(document)
    , m_options(document)
    , m_token(m_options.useThreading ? nullptr : adoptPtr(new HTMLToken))
    , m_tokenizer(m_options.useThreading ? nullptr : HTMLTokenizer::create(m_options))
    , m_treeBuilder(HTMLTreeBuilder::create(this, document, parserContentPolicy(), reportErrors, m_options))
    , m_parserScheduler(HTMLParserScheduler::create(this))
    , m_xssAuditorDelegate(document)
#if ENABLE(THREADED_HTML_PARSER)
    , m_weakFactory(this)
#endif
    , m_preloader(adoptPtr(new HTMLResourcePreloader(document)))
    , m_isPinnedToMainThread(false)
    , m_endWasDelayed(false)
    , m_haveBackgroundParser(false)
    , m_pumpSessionNestingLevel(0)
{
    ASSERT(shouldUseThreading() || (m_token && m_tokenizer));
}

// FIXME: Member variables should be grouped into self-initializing structs to
// minimize code duplication between these constructors.
HTMLDocumentParser::HTMLDocumentParser(DocumentFragment* fragment, Element* contextElement, ParserContentPolicy parserContentPolicy)
    : ScriptableDocumentParser(fragment->document(), parserContentPolicy)
    , m_options(fragment->document())
    , m_token(adoptPtr(new HTMLToken))
    , m_tokenizer(HTMLTokenizer::create(m_options))
    , m_treeBuilder(HTMLTreeBuilder::create(this, fragment, contextElement, this->parserContentPolicy(), m_options))
    , m_xssAuditorDelegate(fragment->document())
#if ENABLE(THREADED_HTML_PARSER)
    , m_weakFactory(this)
#endif
    , m_isPinnedToMainThread(true)
    , m_endWasDelayed(false)
    , m_haveBackgroundParser(false)
    , m_pumpSessionNestingLevel(0)
{
    ASSERT(!shouldUseThreading());
    bool reportErrors = false; // For now document fragment parsing never reports errors.
    m_tokenizer->setState(tokenizerStateForContextElement(contextElement, reportErrors, m_options));
    m_xssAuditor.initForFragment();
}

HTMLDocumentParser::~HTMLDocumentParser()
{
    ASSERT(!m_parserScheduler);
    ASSERT(!m_pumpSessionNestingLevel);
    ASSERT(!m_preloadScanner);
    ASSERT(!m_insertionPreloadScanner);
    ASSERT(!m_haveBackgroundParser);
}

#if ENABLE(THREADED_HTML_PARSER)
void HTMLDocumentParser::pinToMainThread()
{
    ASSERT(!m_haveBackgroundParser);
    ASSERT(!m_isPinnedToMainThread);
    m_isPinnedToMainThread = true;
    if (!m_tokenizer) {
        ASSERT(!m_token);
        m_token = adoptPtr(new HTMLToken);
        m_tokenizer = HTMLTokenizer::create(m_options);
    }
}
#endif

void HTMLDocumentParser::detach()
{
#if ENABLE(THREADED_HTML_PARSER)
    if (m_haveBackgroundParser)
        stopBackgroundParser();
#endif
    DocumentParser::detach();
    m_treeBuilder->detach();
    // FIXME: It seems wrong that we would have a preload scanner here.
    // Yet during fast/dom/HTMLScriptElement/script-load-events.html we do.
    m_preloadScanner.clear();
    m_insertionPreloadScanner.clear();
    m_parserScheduler.clear(); // Deleting the scheduler will clear any timers.
}

void HTMLDocumentParser::stopParsing()
{
    DocumentParser::stopParsing();
    m_parserScheduler.clear(); // Deleting the scheduler will clear any timers.
#if ENABLE(THREADED_HTML_PARSER)
    if (m_haveBackgroundParser)
        stopBackgroundParser();
#endif
}

// This kicks off "Once the user agent stops parsing" as described by:
// http://www.whatwg.org/specs/web-apps/current-work/multipage/the-end.html#the-end
void HTMLDocumentParser::prepareToStopParsing()
{
    // FIXME: It may not be correct to disable this for the background parser.
    // That means hasInsertionPoint() may not be correct in some cases.
    ASSERT(!hasInsertionPoint() || m_haveBackgroundParser);

    // pumpTokenizer can cause this parser to be detached from the Document,
    // but we need to ensure it isn't deleted yet.
    RefPtr<HTMLDocumentParser> protect(this);

    // NOTE: This pump should only ever emit buffered character tokens,
    // so ForceSynchronous vs. AllowYield should be meaningless.
#if ENABLE(THREADED_HTML_PARSER)
    if (m_tokenizer) {
        ASSERT(!m_haveBackgroundParser);
        pumpTokenizerIfPossible(ForceSynchronous);
    }
#else
    pumpTokenizerIfPossible(ForceSynchronous);
#endif

    if (isStopped())
        return;

    DocumentParser::prepareToStopParsing();

    // Setting the ready state above can fire mutation event and detach us
    // from underneath. In that case, just bail out.
    if (isDetached())
        return;
}

bool HTMLDocumentParser::isParsingFragment() const
{
    return m_treeBuilder->isParsingFragment();
}

bool HTMLDocumentParser::processingData() const
{
    return isScheduledForResume() || inPumpSession() || m_haveBackgroundParser;
}

void HTMLDocumentParser::pumpTokenizerIfPossible(SynchronousMode mode)
{
    if (isStopped())
        return;

    // Once a resume is scheduled, HTMLParserScheduler controls when we next pump.
    if (isScheduledForResume()) {
        ASSERT(mode == AllowYield);
        return;
    }

    pumpTokenizer(mode);
}

bool HTMLDocumentParser::isScheduledForResume() const
{
    return m_parserScheduler && m_parserScheduler->isScheduledForResume();
}

// Used by HTMLParserScheduler
void HTMLDocumentParser::resumeParsingAfterYield()
{
    // pumpTokenizer can cause this parser to be detached from the Document,
    // but we need to ensure it isn't deleted yet.
    RefPtr<HTMLDocumentParser> protect(this);

#if ENABLE(THREADED_HTML_PARSER)
    if (m_haveBackgroundParser) {
        pumpPendingSpeculations();
        return;
    }
#endif

    // We should never be here unless we can pump immediately.  Call pumpTokenizer()
    // directly so that ASSERTS will fire if we're wrong.
    pumpTokenizer(AllowYield);
    endIfDelayed();
}

bool HTMLDocumentParser::canTakeNextToken(SynchronousMode mode, PumpSession& session)
{
    if (isStopped())
        return false;

    ASSERT(!m_haveBackgroundParser || mode == ForceSynchronous);

    if (mode == AllowYield)
        m_parserScheduler->checkForYieldBeforeToken(session);

    return true;
}

#if ENABLE(THREADED_HTML_PARSER)

void HTMLDocumentParser::didReceiveParsedChunkFromBackgroundParser(PassOwnPtr<ParsedChunk> chunk)
{
    if (!m_speculations.isEmpty()) {
        m_preloader->takeAndPreload(chunk->preloads);
        m_speculations.append(chunk);
        return;
    }

    // processParsedChunkFromBackgroundParser can cause this parser to be detached from the Document,
    // but we need to ensure it isn't deleted yet.
    RefPtr<HTMLDocumentParser> protect(this);


    ASSERT(m_speculations.isEmpty());
    chunk->preloads.clear(); // We don't need to preload because we're going to parse immediately.
    processParsedChunkFromBackgroundParser(chunk);
}

void HTMLDocumentParser::validateSpeculations(PassOwnPtr<ParsedChunk> chunk)
{
    ASSERT(chunk);

    ASSERT(!m_lastChunkBeforeScript);
    OwnPtr<HTMLTokenizer> tokenizer = m_tokenizer.release();
    OwnPtr<HTMLToken> token = m_token.release();

    if (!tokenizer) {
        // There must not have been any changes to the HTMLTokenizer state on
        // the main thread, which means the speculation buffer is correct.
        return;
    }

    // Currently we're only smart enough to reuse the speculation buffer if the tokenizer
    // both starts and ends in the DataState. That state is simplest because the HTMLToken
    // is always in the Uninitialized state. We should consider whether we can reuse the
    // speculation buffer in other states, but we'd likely need to do something more
    // sophisticated with the HTMLToken.
    if (chunk->tokenizerState == HTMLTokenizer::DataState
        && tokenizer->state() == HTMLTokenizer::DataState
        && m_input.current().isEmpty()
        && chunk->treeBuilderState == HTMLTreeBuilderSimulator::stateFor(m_treeBuilder.get())) {
        ASSERT(token->isUninitialized());
        return;
    }

    discardSpeculationsAndResumeFrom(chunk, token.release(), tokenizer.release());
}

void HTMLDocumentParser::discardSpeculationsAndResumeFrom(PassOwnPtr<ParsedChunk> lastChunkBeforeScript, PassOwnPtr<HTMLToken> token, PassOwnPtr<HTMLTokenizer> tokenizer)
{
    m_weakFactory.revokeAll();
    m_speculations.clear();

    OwnPtr<BackgroundHTMLParser::Checkpoint> checkpoint = adoptPtr(new BackgroundHTMLParser::Checkpoint);
    checkpoint->parser = m_weakFactory.createWeakPtr();
    checkpoint->token = token;
    checkpoint->tokenizer = tokenizer;
    checkpoint->treeBuilderState = HTMLTreeBuilderSimulator::stateFor(m_treeBuilder.get());
    checkpoint->inputCheckpoint = lastChunkBeforeScript->inputCheckpoint;
    checkpoint->preloadScannerCheckpoint = lastChunkBeforeScript->preloadScannerCheckpoint;
    checkpoint->unparsedInput = m_input.current().toString().isolatedCopy();
    m_input.current().clear(); // FIXME: This should be passed in instead of cleared.

    ASSERT(checkpoint->unparsedInput.isSafeToSendToAnotherThread());
    HTMLParserThread::shared()->postTask(bind(&BackgroundHTMLParser::resumeFrom, m_backgroundParser, checkpoint.release()));
}

void HTMLDocumentParser::processParsedChunkFromBackgroundParser(PassOwnPtr<ParsedChunk> popChunk)
{
    // ASSERT that this object is both attached to the Document and protected.
    ASSERT(refCount() >= 2);
    ASSERT(shouldUseThreading());
    ASSERT(!m_tokenizer);
    ASSERT(!m_token);
    ASSERT(!m_lastChunkBeforeScript);

    ActiveParserSession session(contextForParsingSession());
    OwnPtr<ParsedChunk> chunk(popChunk);
    OwnPtr<CompactHTMLTokenStream> tokens = chunk->tokens.release();

    HTMLParserThread::shared()->postTask(bind(&BackgroundHTMLParser::startedChunkWithCheckpoint, m_backgroundParser, chunk->inputCheckpoint));

    for (XSSInfoStream::const_iterator it = chunk->xssInfos.begin(); it != chunk->xssInfos.end(); ++it) {
        m_textPosition = (*it)->m_textPosition;
        m_xssAuditorDelegate.didBlockScript(**it); 
        if (isStopped())
            break;
    }

    for (Vector<CompactHTMLToken>::const_iterator it = tokens->begin(); it != tokens->end(); ++it) {

        m_textPosition = it->textPosition();

        constructTreeFromCompactHTMLToken(*it);

        if (isStopped())
            break;

        if (it->type() == HTMLToken::EndOfFile) {
            ASSERT(it + 1 == tokens->end()); // The EOF is assumed to be the last token of this bunch.
            ASSERT(m_speculations.isEmpty());
            prepareToStopParsing();
            break;
        }

        ASSERT(!m_tokenizer);
        ASSERT(!m_token);
    }
}

void HTMLDocumentParser::pumpPendingSpeculations()
{
    // FIXME: Share this constant with the parser scheduler.
    const double parserTimeLimit = 0.500;

    // ASSERT that this object is both attached to the Document and protected.
    ASSERT(refCount() >= 2);
    // If this assert fails, you need to call validateSpeculations to make sure
    // m_tokenizer and m_token don't have state that invalidates m_speculations.
    ASSERT(!m_tokenizer);
    ASSERT(!m_token);
    ASSERT(!m_lastChunkBeforeScript);

    double startTime = currentTime();

    while (!m_speculations.isEmpty()) {
        processParsedChunkFromBackgroundParser(m_speculations.takeFirst());

        if (isStopped())
            break;

        if (currentTime() - startTime > parserTimeLimit && !m_speculations.isEmpty()) {
            m_parserScheduler->scheduleForResume();
            break;
        }
    }
}

#endif // ENABLE(THREADED_HTML_PARSER)

void HTMLDocumentParser::forcePlaintextForTextDocument()
{
#if ENABLE(THREADED_HTML_PARSER)
    if (shouldUseThreading()) {
        // This method is called before any data is appended, so we have to start
        // the background parser ourselves.
        if (!m_haveBackgroundParser)
            startBackgroundParser();

        HTMLParserThread::shared()->postTask(bind(&BackgroundHTMLParser::forcePlaintextForTextDocument, m_backgroundParser));
    } else
#endif
        m_tokenizer->setState(HTMLTokenizer::PLAINTEXTState);
}

Document* HTMLDocumentParser::contextForParsingSession()
{
    // The parsing session should interact with the document only when parsing
    // non-fragments. Otherwise, we might delay the load event mistakenly.
    if (isParsingFragment())
        return 0;
    return document();
}

void HTMLDocumentParser::pumpTokenizer(SynchronousMode mode)
{
    ASSERT(!isStopped());
    ASSERT(!isScheduledForResume());
    // ASSERT that this object is both attached to the Document and protected.
    ASSERT(refCount() >= 2);
    ASSERT(m_tokenizer);
    ASSERT(m_token);
    ASSERT(!m_haveBackgroundParser || mode == ForceSynchronous);

    PumpSession session(m_pumpSessionNestingLevel, contextForParsingSession());


    m_xssAuditor.init(document(), &m_xssAuditorDelegate);

    while (canTakeNextToken(mode, session) && !session.needsYield) {
        if (!isParsingFragment())
            m_sourceTracker.start(m_input.current(), m_tokenizer.get(), token());

        if (!m_tokenizer->nextToken(m_input.current(), token()))
            break;

        if (!isParsingFragment()) {
            m_sourceTracker.end(m_input.current(), m_tokenizer.get(), token());

            // We do not XSS filter innerHTML, which means we (intentionally) fail
            // http/tests/security/xssAuditor/dom-write-innerHTML.html
            if (OwnPtr<XSSInfo> xssInfo = m_xssAuditor.filterToken(FilterTokenRequest(token(), m_sourceTracker, m_tokenizer->shouldAllowCDATA())))
                m_xssAuditorDelegate.didBlockScript(*xssInfo);
        }

        constructTreeFromHTMLToken(token());
        ASSERT(token().isUninitialized());
    }

    // Ensure we haven't been totally deref'ed after pumping. Any caller of this
    // function should be holding a RefPtr to this to ensure we weren't deleted.
    ASSERT(refCount() >= 1);

    if (isStopped())
        return;

    if (session.needsYield)
        m_parserScheduler->scheduleForResume();

}

void HTMLDocumentParser::constructTreeFromHTMLToken(HTMLToken& rawToken)
{
    AtomicHTMLToken token(rawToken);

    // We clear the rawToken in case constructTreeFromAtomicToken
    // synchronously re-enters the parser. We don't clear the token immedately
    // for Character tokens because the AtomicHTMLToken avoids copying the
    // characters by keeping a pointer to the underlying buffer in the
    // HTMLToken. Fortunately, Character tokens can't cause us to re-enter
    // the parser.
    //
    // FIXME: Stop clearing the rawToken once we start running the parser off
    // the main thread or once we stop allowing synchronous JavaScript
    // execution from parseAttribute.
    if (rawToken.type() != HTMLToken::Character)
        rawToken.clear();

    m_treeBuilder->constructTree(&token);

    if (!rawToken.isUninitialized()) {
        ASSERT(rawToken.type() == HTMLToken::Character);
        rawToken.clear();
    }
}

#if ENABLE(THREADED_HTML_PARSER)

void HTMLDocumentParser::constructTreeFromCompactHTMLToken(const CompactHTMLToken& compactToken)
{
    AtomicHTMLToken token(compactToken);
    m_treeBuilder->constructTree(&token);
}

#endif

bool HTMLDocumentParser::hasInsertionPoint()
{
    // FIXME: The wasCreatedByScript() branch here might not be fully correct.
    //        Our model of the EOF character differs slightly from the one in
    //        the spec because our treatment is uniform between network-sourced
    //        and script-sourced input streams whereas the spec treats them
    //        differently.
    return m_input.hasInsertionPoint() || (wasCreatedByScript() && !m_input.haveSeenEndOfFile());
}

void HTMLDocumentParser::insert(const SegmentedString& source)
{
    if (isStopped())
        return;

    // pumpTokenizer can cause this parser to be detached from the Document,
    // but we need to ensure it isn't deleted yet.
    RefPtr<HTMLDocumentParser> protect(this);

#if ENABLE(THREADED_HTML_PARSER)
    if (!m_tokenizer) {
        ASSERT(!inPumpSession());
        ASSERT(m_haveBackgroundParser || wasCreatedByScript());
        m_token = adoptPtr(new HTMLToken);
        m_tokenizer = HTMLTokenizer::create(m_options);
    }
#endif

    SegmentedString excludedLineNumberSource(source);
    excludedLineNumberSource.setExcludeLineNumbers();
    m_input.insertAtCurrentInsertionPoint(excludedLineNumberSource);
    pumpTokenizerIfPossible(ForceSynchronous);
    endIfDelayed();
}

#if ENABLE(THREADED_HTML_PARSER)

void HTMLDocumentParser::startBackgroundParser()
{
    ASSERT(shouldUseThreading());
    ASSERT(!m_haveBackgroundParser);
    m_haveBackgroundParser = true;

    HTMLIdentifier::init();

    RefPtr<WeakReference<BackgroundHTMLParser> > reference = WeakReference<BackgroundHTMLParser>::createUnbound();
    m_backgroundParser = WeakPtr<BackgroundHTMLParser>(reference);

    OwnPtr<BackgroundHTMLParser::Configuration> config = adoptPtr(new BackgroundHTMLParser::Configuration);
    config->options = m_options;
    config->parser = m_weakFactory.createWeakPtr();
    config->xssAuditor = adoptPtr(new XSSAuditor);
    config->xssAuditor->init(document(), &m_xssAuditorDelegate);
    config->preloadScanner = adoptPtr(new TokenPreloadScanner(document()->url().copy()));

    ASSERT(config->xssAuditor->isSafeToSendToAnotherThread());
    ASSERT(config->preloadScanner->isSafeToSendToAnotherThread());
    HTMLParserThread::shared()->postTask(bind(&BackgroundHTMLParser::create, reference.release(), config.release()));
}

void HTMLDocumentParser::stopBackgroundParser()
{
    ASSERT(shouldUseThreading());
    ASSERT(m_haveBackgroundParser);
    m_haveBackgroundParser = false;

    HTMLParserThread::shared()->postTask(bind(&BackgroundHTMLParser::stop, m_backgroundParser));
    m_weakFactory.revokeAll();
}

#endif

void HTMLDocumentParser::append(PassRefPtr<StringImpl> inputSource)
{
    if (isStopped())
        return;

#if ENABLE(THREADED_HTML_PARSER)
    if (shouldUseThreading()) {
        if (!m_haveBackgroundParser)
            startBackgroundParser();

        ASSERT(inputSource->hasOneRef());
        Closure closure = bind(&BackgroundHTMLParser::append, m_backgroundParser, String(inputSource));
        // NOTE: Important that the String temporary is destroyed before we post the task
        // otherwise the String could call deref() on a StringImpl now owned by the background parser.
        // We would like to ASSERT(closure.arg3()->hasOneRef()) but sadly the args are private.
        HTMLParserThread::shared()->postTask(closure);
        return;
    }
#endif

    // pumpTokenizer can cause this parser to be detached from the Document,
    // but we need to ensure it isn't deleted yet.
    RefPtr<HTMLDocumentParser> protect(this);
    String source(inputSource);

    if (m_preloadScanner) {
        if (m_input.current().isEmpty()) {
            // We have parsed until the end of the current input and so are now moving ahead of the preload scanner.
            // Clear the scanner so we know to scan starting from the current input point if we block again.
            m_preloadScanner.clear();
        } else {
            m_preloadScanner->appendToEnd(source);
        }
    }

    m_input.appendToEnd(source);

    if (inPumpSession()) {
        // We've gotten data off the network in a nested write.
        // We don't want to consume any more of the input stream now.  Do
        // not worry.  We'll consume this data in a less-nested write().
        return;
    }

    pumpTokenizerIfPossible(AllowYield);

    endIfDelayed();
}

void HTMLDocumentParser::end()
{
    ASSERT(!isDetached());
    ASSERT(!isScheduledForResume());

#if ENABLE(THREADED_HTML_PARSER)
    if (m_haveBackgroundParser)
        stopBackgroundParser();
#endif

    // Informs the the rest of WebCore that parsing is really finished (and deletes this).
    m_treeBuilder->finished();
}

void HTMLDocumentParser::attemptToEnd()
{
    // finish() indicates we will not receive any more data. If we are waiting on
    // an external script to load, we can't finish parsing quite yet.

    if (shouldDelayEnd()) {
        m_endWasDelayed = true;
        return;
    }
    prepareToStopParsing();
}

void HTMLDocumentParser::endIfDelayed()
{
    // If we've already been detached, don't bother ending.
    if (isDetached())
        return;

    if (!m_endWasDelayed || shouldDelayEnd())
        return;

    m_endWasDelayed = false;
    prepareToStopParsing();
}

void HTMLDocumentParser::finish()
{
    // FIXME: We should ASSERT(!m_parserStopped) here, since it does not
    // makes sense to call any methods on DocumentParser once it's been stopped.
    // However, FrameLoader::stop calls DocumentParser::finish unconditionally.

#if ENABLE(THREADED_HTML_PARSER)
    // Empty documents never got an append() call, and thus have never started
    // a background parser. In those cases, we ignore shouldUseThreading()
    // and fall through to the non-threading case.
    if (m_haveBackgroundParser) {
        if (!m_input.haveSeenEndOfFile())
            m_input.closeWithoutMarkingEndOfFile();
        HTMLParserThread::shared()->postTask(bind(&BackgroundHTMLParser::finish, m_backgroundParser));
        return;
    }

    if (!m_tokenizer) {
        ASSERT(!m_token);
        // We're finishing before receiving any data. Rather than booting up
        // the background parser just to spin it down, we finish parsing
        // synchronously.
        m_token = adoptPtr(new HTMLToken);
        m_tokenizer = HTMLTokenizer::create(m_options);
    }
#endif

    // We're not going to get any more data off the network, so we tell the
    // input stream we've reached the end of file. finish() can be called more
    // than once, if the first time does not call end().
    if (!m_input.haveSeenEndOfFile())
        m_input.markEndOfFile();

    attemptToEnd();
}

OrdinalNumber HTMLDocumentParser::lineNumber() const
{
#if ENABLE(THREADED_HTML_PARSER)
    if (m_haveBackgroundParser)
        return m_textPosition.m_line;
#endif

    return m_input.current().currentLine();
}

TextPosition HTMLDocumentParser::textPosition() const
{
#if ENABLE(THREADED_HTML_PARSER)
    if (m_haveBackgroundParser)
        return m_textPosition;
#endif

    const SegmentedString& currentString = m_input.current();
    OrdinalNumber line = currentString.currentLine();
    OrdinalNumber column = currentString.currentColumn();

    return TextPosition(line, column);
}

void HTMLDocumentParser::notifyFinished(CachedResource* cachedResource)
{
    // pumpTokenizer can cause this parser to be detached from the Document,
    // but we need to ensure it isn't deleted yet.
    RefPtr<HTMLDocumentParser> protect(this);

    if (isStopping()) {
        return;
    }
}

void HTMLDocumentParser::parseDocumentFragment(const String& source, DocumentFragment* fragment, Element* contextElement, ParserContentPolicy parserContentPolicy)
{
    RefPtr<HTMLDocumentParser> parser = HTMLDocumentParser::create(fragment, contextElement, parserContentPolicy);
    parser->insert(source); // Use insert() so that the parser will not yield.
    parser->finish();
    ASSERT(!parser->processingData()); // Make sure we're done. <rdar://problem/3963151>
    parser->detach(); // Allows ~DocumentParser to assert it was detached before destruction.
}
    
void HTMLDocumentParser::suspendScheduledTasks()
{
    if (m_parserScheduler)
        m_parserScheduler->suspend();
}

void HTMLDocumentParser::resumeScheduledTasks()
{
    if (m_parserScheduler)
        m_parserScheduler->resume();
}

}
