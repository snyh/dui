/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 * Copyright (C) 2011 Kris Jordan <krisjordan@gmail.com>
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "loader/FrameLoader.h"

#include "accessibility/AXObjectCache.h"
#include "dom/BeforeUnloadEvent.h"
#include "loader/cache/CachedResourceLoader.h"
#include "dom/DOMImplementation.h"
#include "page/DOMWindow.h"
#include "dom/Document.h"
#include "loader/DocumentLoadTiming.h"
#include "loader/DocumentLoader.h"
#include "editing/Editor.h"
#include "page/EditorClient.h"
#include "dom/Element.h"
#include "dom/Event.h"
#include "page/EventHandler.h"
#include "dom/EventNames.h"
#include "platform/graphics/FloatRect.h"
#include "loader/FormState.h"
#include "loader/FormSubmission.h"
#include "page/Frame.h"
#include "html/HTMLFrameOwnerElement.h"
#include "loader/FrameLoadRequest.h"
#include "loader/FrameLoaderClient.h"
#include "loader/FrameNetworkingContext.h"
#include "page/FrameTree.h"
#include "page/FrameView.h"
#include "html/HTMLAnchorElement.h"
#include "html/HTMLFormElement.h"
#include "html/HTMLInputElement.h"
#include "HTMLNames.h"
#include "html/parser/HTMLParserIdioms.h"
#include "platform/network/HTTPParsers.h"
#include "platform/Logging.h"
#include "platform/MIMETypeRegistry.h"
#include "loader/cache/MemoryCache.h"
#include "page/Page.h"
#include "dom/PageTransitionEvent.h"
#include "loader/PolicyChecker.h"
#include "loader/ProgressTracker.h"
#include "platform/network/ResourceHandle.h"
#include "platform/network/soup/ResourceRequest.h"
#include "platform/SchemeRegistry.h"
#include "platform/ScrollAnimator.h"
#include "platform/text/SegmentedString.h"
#include "bindings/dui/SerializedScriptValue.h"
#include "page/Settings.h"
#include "loader/TextResourceDecoder.h"
#include "page/WindowFeatures.h"
#include "xml/parser/XMLDocumentParser.h"
#include <wtf/CurrentTime.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

#if ENABLE(SVG)
#include "svg/SVGDocument.h"
#include "svg/SVGLocatable.h"
#include "SVGNames.h"
#include "svg/SVGPreserveAspectRatio.h"
#include "svg/SVGSVGElement.h"
#include "svg/SVGViewElement.h"
#include "svg/SVGViewSpec.h"
#endif

namespace WebCore {

using namespace HTMLNames;

#if ENABLE(SVG)
using namespace SVGNames;
#endif

static const char defaultAcceptHeader[] = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
static double storedTimeOfLastCompletedLoad;

// This is not in the FrameLoader class to emphasize that it does not depend on
// private FrameLoader data, and to avoid increasing the number of public functions
// with access to private data.  Since only this .cpp file needs it, making it
// non-member lets us exclude it from the header file, thus keeping FrameLoader.h's
// API simpler.
//
class FrameLoader::FrameProgressTracker {
public:
    static PassOwnPtr<FrameProgressTracker> create(Frame* frame) { return adoptPtr(new FrameProgressTracker(frame)); }
    ~FrameProgressTracker()
    {
        ASSERT(!m_inProgress || m_frame->page());
        if (m_inProgress)
            m_frame->page()->progress()->progressCompleted(m_frame);
    }

    void progressStarted()
    {
        ASSERT(m_frame->page());
        if (!m_inProgress)
            m_frame->page()->progress()->progressStarted(m_frame);
        m_inProgress = true;
    }

    void progressCompleted()
    {
        ASSERT(m_inProgress);
        ASSERT(m_frame->page());
        m_inProgress = false;
        m_frame->page()->progress()->progressCompleted(m_frame);
    }

private:
    FrameProgressTracker(Frame* frame)
        : m_frame(frame)
        , m_inProgress(false)
    {
    }

    Frame* m_frame;
    bool m_inProgress;
};

FrameLoader::FrameLoader(Frame* frame, FrameLoaderClient* client)
    : m_frame(frame)
    , m_client(client)
    , m_policyChecker(adoptPtr(new PolicyChecker(frame)))
    , m_notifer(frame)
    , m_subframeLoader(frame)
    , m_state(FrameStateProvisional)
    , m_loadType(FrameLoadTypeStandard)
    , m_delegateIsHandlingProvisionalLoadError(false)
    , m_quickRedirectComing(false)
    , m_sentRedirectNotification(false)
    , m_inStopAllLoaders(false)
    , m_isExecutingJavaScriptFormAction(false)
    , m_didCallImplicitClose(true)
    , m_wasUnloadEventEmitted(false)
    , m_pageDismissalEventBeingDispatched(NoDismissal)
    , m_isComplete(false)
    , m_needsClear(false)
    , m_checkTimer(this, &FrameLoader::checkTimerFired)
    , m_shouldCallCheckCompleted(false)
    , m_shouldCallCheckLoadComplete(false)
    , m_opener(0)
    , m_didPerformFirstNavigation(false)
    , m_suppressOpenerInNewFrame(false)
{
}

FrameLoader::~FrameLoader()
{
    setOpener(0);

    HashSet<Frame*>::iterator end = m_openedFrames.end();
    for (HashSet<Frame*>::iterator it = m_openedFrames.begin(); it != end; ++it)
        (*it)->loader()->m_opener = 0;

    m_client->frameLoaderDestroyed();

    if (m_networkingContext)
        m_networkingContext->invalidate();
}

void FrameLoader::init()
{
    // This somewhat odd set of steps gives the frame an initial empty document.
    printf("1:%p\n", m_client);
    //setPolicyDocumentLoader(m_client->createDocumentLoader(ResourceRequest(KURL(ParsedURLString, emptyString())), SubstituteData()).get());
    printf("2\n");
    setProvisionalDocumentLoader(m_policyDocumentLoader.get());
    printf("3\n");
    //m_provisionalDocumentLoader->startLoadingMainResource();
    printf("4\n");
    //m_frame->document()->cancelParsing();
    printf("5\n");
    m_stateMachine.advanceTo(FrameLoaderStateMachine::DisplayingInitialEmptyDocument);
    printf("6\n");

    //m_networkingContext = m_client->createNetworkingContext();
    printf("7\n");
    m_progressTracker = FrameProgressTracker::create(m_frame);
    printf("8\n");
}

void FrameLoader::changeLocation(const KURL& url, const String& referrer, bool lockHistory, bool lockBackForwardList, bool refresh)
{
    urlSelected(FrameLoadRequest(ResourceRequest(url, referrer, refresh ? ReloadIgnoringCacheData : UseProtocolCachePolicy), "_self"),
        0, lockHistory, lockBackForwardList, MaybeSendReferrer, ReplaceDocumentIfJavaScriptURL);
}

void FrameLoader::urlSelected(const KURL& url, const String& passedTarget, PassRefPtr<Event> triggeringEvent, bool lockHistory, bool lockBackForwardList, ShouldSendReferrer shouldSendReferrer)
{
    urlSelected(FrameLoadRequest(ResourceRequest(url), passedTarget),
        triggeringEvent, lockHistory, lockBackForwardList, shouldSendReferrer, DoNotReplaceDocumentIfJavaScriptURL);
}

// The shouldReplaceDocumentIfJavaScriptURL parameter will go away when the FIXME to eliminate the
// corresponding parameter from ScriptController::executeIfJavaScriptURL() is addressed.
void FrameLoader::urlSelected(const FrameLoadRequest& passedRequest, PassRefPtr<Event> triggeringEvent, bool lockHistory, bool lockBackForwardList, ShouldSendReferrer shouldSendReferrer, ShouldReplaceDocumentIfJavaScriptURL shouldReplaceDocumentIfJavaScriptURL)
{
    ASSERT(!m_suppressOpenerInNewFrame);

    RefPtr<Frame> protect(m_frame);
    FrameLoadRequest frameRequest(passedRequest);

    if (frameRequest.frameName().isEmpty())
        frameRequest.setFrameName(m_frame->document()->baseTarget());

    if (shouldSendReferrer == NeverSendReferrer)
        m_suppressOpenerInNewFrame = true;
    addHTTPOriginIfNeeded(frameRequest.resourceRequest(), outgoingOrigin());

    m_suppressOpenerInNewFrame = false;
}

void FrameLoader::submitForm(PassRefPtr<FormSubmission> submission)
{
    ASSERT(submission->method() == FormSubmission::PostMethod || submission->method() == FormSubmission::GetMethod);

    // FIXME: Find a good spot for these.
    ASSERT(submission->data());
    ASSERT(submission->state());
    ASSERT(!submission->state()->sourceDocument()->frame() || submission->state()->sourceDocument()->frame() == m_frame);
    
    if (!m_frame->page())
        return;
    
    if (submission->action().isEmpty())
        return;

    Frame* targetFrame = findFrameForNavigation(submission->target(), submission->state()->sourceDocument());
    if (!targetFrame) {
        targetFrame = m_frame;
    } else
        submission->clearTarget();

    if (!targetFrame->page())
        return;

    // FIXME: We'd like to remove this altogether and fix the multiple form submission issue another way.

    // We do not want to submit more than one form from the same page, nor do we want to submit a single
    // form more than once. This flag prevents these from happening; not sure how other browsers prevent this.
    // The flag is reset in each time we start handle a new mouse or key down event, and
    // also in setView since this part may get reused for a page from the back/forward cache.
    // The form multi-submit logic here is only needed when we are submitting a form that affects this frame.

    // FIXME: Frame targeting is only one of the ways the submission could end up doing something other
    // than replacing this frame's content, so this check is flawed. On the other hand, the check is hardly
    // needed any more now that we reset m_submittedFormURL on each mouse or key down event.

    if (m_frame->tree()->isDescendantOf(targetFrame)) {
        if (m_submittedFormURL == submission->requestURL())
            return;
        m_submittedFormURL = submission->requestURL();
    }

    submission->data()->generateFiles(m_frame->document());
    submission->setReferrer(outgoingReferrer());
    submission->setOrigin(outgoingOrigin());
}

void FrameLoader::stopLoading(UnloadEventPolicy unloadEventPolicy)
{
    if (m_frame->document() && m_frame->document()->parser())
        m_frame->document()->parser()->stopParsing();

    if (unloadEventPolicy != UnloadEventPolicyNone) {
        if (m_frame->document()) {
            if (m_didCallImplicitClose && !m_wasUnloadEventEmitted) {
                Element* currentFocusedElement = m_frame->document()->focusedElement();
                if (currentFocusedElement && currentFocusedElement->toInputElement())
                    currentFocusedElement->toInputElement()->endEditing();
                if (m_pageDismissalEventBeingDispatched == NoDismissal) {
                    if (unloadEventPolicy == UnloadEventPolicyUnloadAndPageHide) {
                        m_pageDismissalEventBeingDispatched = PageHideDismissal;
                        m_frame->document()->domWindow()->dispatchEvent(PageTransitionEvent::create(eventNames().pagehideEvent, m_frame->document()->inPageCache()), m_frame->document());
                    }

                    // FIXME: update Page Visibility state here.
                    // https://bugs.webkit.org/show_bug.cgi?id=116770

                    if (!m_frame->document()->inPageCache()) {
                        RefPtr<Event> unloadEvent(Event::create(eventNames().unloadEvent, false, false));
                        // The DocumentLoader (and thus its DocumentLoadTiming) might get destroyed
                        // while dispatching the event, so protect it to prevent writing the end
                        // time into freed memory.
                        RefPtr<DocumentLoader> documentLoader = m_provisionalDocumentLoader;
                        m_pageDismissalEventBeingDispatched = UnloadDismissal;
                        if (documentLoader && !documentLoader->timing()->unloadEventStart() && !documentLoader->timing()->unloadEventEnd()) {
                            DocumentLoadTiming* timing = documentLoader->timing();
                            ASSERT(timing->navigationStart());
                            timing->markUnloadEventStart();
                            m_frame->document()->domWindow()->dispatchEvent(unloadEvent, m_frame->document());
                            timing->markUnloadEventEnd();
                        } else
                            m_frame->document()->domWindow()->dispatchEvent(unloadEvent, m_frame->document());
                    }
                }
                m_pageDismissalEventBeingDispatched = NoDismissal;
                if (m_frame->document())
                    m_frame->document()->updateStyleIfNeeded();
                m_wasUnloadEventEmitted = true;
            }
        }

        // Dispatching the unload event could have made m_frame->document() null.
        if (m_frame->document() && !m_frame->document()->inPageCache()) {
            // Don't remove event listeners from a transitional empty document (see bug 28716 for more information).
            bool keepEventListeners = m_stateMachine.isDisplayingInitialEmptyDocument() && m_provisionalDocumentLoader;

            if (!keepEventListeners)
                m_frame->document()->removeAllEventListeners();
        }
    }

    m_isComplete = true; // to avoid calling completed() in finishedParsing()
    m_didCallImplicitClose = true; // don't want that one either

    if (m_frame->document() && m_frame->document()->parsing()) {
        finishedParsing();
        m_frame->document()->setParsing(false);
    }

    if (Document* doc = m_frame->document()) {
        // FIXME: HTML5 doesn't tell us to set the state to complete when aborting, but we do anyway to match legacy behavior.
        // http://www.w3.org/Bugs/Public/show_bug.cgi?id=10537
        doc->setReadyState(Document::Complete);

    }
}

void FrameLoader::stop()
{
    // http://bugs.webkit.org/show_bug.cgi?id=10854
    // The frame's last ref may be removed and it will be deleted by checkCompleted().
    RefPtr<Frame> protector(m_frame);

    if (DocumentParser* parser = m_frame->document()->parser()) {
        parser->stopParsing();
        parser->finish();
    }
}

void FrameLoader::willTransitionToCommitted()
{
    // This function is called when a frame is still fully in place (not cached, not detached), but will be replaced.

    if (m_frame->editor().hasComposition()) {
        // The text was already present in DOM, so it's better to confirm than to cancel the composition.
        m_frame->editor().confirmComposition();
        if (EditorClient* editorClient = m_frame->editor().client())
            editorClient->respondToChangedSelection(m_frame);
    }
}

bool FrameLoader::closeURL()
{
    // Should only send the pagehide event here if the current document exists and has not been placed in the page cache.    
    Document* currentDocument = m_frame->document();
    stopLoading(currentDocument && !currentDocument->inPageCache() ? UnloadEventPolicyUnloadAndPageHide : UnloadEventPolicyUnloadOnly);
    
    m_frame->editor().clearUndoRedoOperations();
    return true;
}

bool FrameLoader::didOpenURL()
{
    m_frame->editor().clearLastEditCommand();

    m_isComplete = false;
    m_didCallImplicitClose = false;

    started();

    return true;
}

void FrameLoader::didExplicitOpen()
{
    m_isComplete = false;
    m_didCallImplicitClose = false;

    // Calling document.open counts as committing the first real document load.
    if (!m_stateMachine.committedFirstRealDocumentLoad())
        m_stateMachine.advanceTo(FrameLoaderStateMachine::DisplayingInitialEmptyDocumentPostCommit);
}


void FrameLoader::cancelAndClear()
{
    if (!m_isComplete)
        closeURL();

    clear(m_frame->document(), false);
}

void FrameLoader::clear(Document* newDocument, bool clearWindowProperties, bool clearScriptObjects, bool clearFrameView)
{
    m_frame->editor().clear();

    if (!m_needsClear)
        return;
    m_needsClear = false;
    
    if (!m_frame->document()->inPageCache()) {
        m_frame->document()->cancelParsing();
        m_frame->document()->stopActiveDOMObjects();
        if (m_frame->document()->attached()) {
            m_frame->document()->prepareForDestruction();
            m_frame->document()->removeFocusedNodeOfSubtree(m_frame->document());
        }
    }

    // Do this after detaching the document so that the unload event works.
    if (clearWindowProperties) {
        m_frame->document()->domWindow()->resetUnlessSuspendedForPageCache();
    }

    m_frame->selection()->prepareForDestruction();
    m_frame->eventHandler()->clear();
    if (clearFrameView && m_frame->view())
        m_frame->view()->clear();

    // Do not drop the document before the ScriptController and view are cleared
    // as some destructors might still try to access the document.
    m_frame->setDocument(0);

    m_subframeLoader.clear();

    m_checkTimer.stop();
    m_shouldCallCheckCompleted = false;
    m_shouldCallCheckLoadComplete = false;

    if (m_stateMachine.isDisplayingInitialEmptyDocument() && m_stateMachine.committedFirstRealDocumentLoad())
        m_stateMachine.advanceTo(FrameLoaderStateMachine::CommittedFirstRealLoad);
}

void FrameLoader::receivedFirstData()
{
    dispatchDidCommitLoad();
    dispatchDidClearWindowObjectsInAllWorlds();
    dispatchGlobalObjectAvailableInAllWorlds();

    if (m_documentLoader) {
        StringWithDirection ptitle = m_documentLoader->title();
        // If we have a title let the WebView know about it.
        if (!ptitle.isNull())
            m_client->dispatchDidReceiveTitle(ptitle);
    }

    if (!m_documentLoader)
        return;

    double delay;
    String url;
    if (!parseHTTPRefresh(m_documentLoader->response().httpHeaderField("Refresh"), false, delay, url))
        return;
    if (url.isEmpty())
        url = m_frame->document()->url().string();
    else
        url = m_frame->document()->completeURL(url).string();
}

void FrameLoader::setOutgoingReferrer(const KURL& url)
{
    m_outgoingReferrer = url.strippedForUseAsReferrer();
}

void FrameLoader::didBeginDocument(bool dispatch)
{
    m_needsClear = true;
    m_isComplete = false;
    m_didCallImplicitClose = false;
    m_frame->document()->setReadyState(Document::Loading);

    if (m_pendingStateObject) {
        m_frame->document()->statePopped(m_pendingStateObject.get());
        m_pendingStateObject.clear();
    }

    if (dispatch)
        dispatchDidClearWindowObjectsInAllWorlds();

    Settings* settings = m_frame->document()->settings();
    if (settings) {
        m_frame->document()->cachedResourceLoader()->setImagesEnabled(settings->areImagesEnabled());
        m_frame->document()->cachedResourceLoader()->setAutoLoadImages(settings->loadsImagesAutomatically());
    }

    if (m_documentLoader) {
        String headerContentLanguage = m_documentLoader->response().httpHeaderField("Content-Language");
        if (!headerContentLanguage.isEmpty()) {
            size_t commaIndex = headerContentLanguage.find(',');
            headerContentLanguage.truncate(commaIndex); // notFound == -1 == don't truncate
            headerContentLanguage = headerContentLanguage.stripWhiteSpace(isHTMLSpace);
            if (!headerContentLanguage.isEmpty())
                m_frame->document()->setContentLanguage(headerContentLanguage);
        }
    }
}

void FrameLoader::finishedParsing()
{
    if (m_stateMachine.creatingInitialEmptyDocument())
        return;

    // This can be called from the Frame's destructor, in which case we shouldn't protect ourselves
    // because doing so will cause us to re-enter the destructor when protector goes out of scope.
    // Null-checking the FrameView indicates whether or not we're in the destructor.
    RefPtr<Frame> protector = m_frame->view() ? m_frame : 0;

    m_client->dispatchDidFinishDocumentLoad();

    checkCompleted();

    if (!m_frame->view())
        return; // We are being destroyed by something checkCompleted called.

    // Check if the scrollbars are really needed for the content.
    // If not, remove them, relayout, and repaint.
    m_frame->view()->restoreScrollbar();
    scrollToFragmentWithParentBoundary(m_frame->document()->url());
}

void FrameLoader::loadDone()
{
    checkCompleted();
}

bool FrameLoader::allChildrenAreComplete() const
{
    for (Frame* child = m_frame->tree()->firstChild(); child; child = child->tree()->nextSibling()) {
        if (!child->loader()->m_isComplete)
            return false;
    }
    return true;
}

bool FrameLoader::allAncestorsAreComplete() const
{
    for (Frame* ancestor = m_frame; ancestor; ancestor = ancestor->tree()->parent()) {
        if (!ancestor->loader()->m_isComplete)
            return false;
    }
    return true;
}

void FrameLoader::checkCompleted()
{
    RefPtr<Frame> protect(m_frame);
    m_shouldCallCheckCompleted = false;

    if (m_frame->view())
        m_frame->view()->handleLoadCompleted();

    // Have we completed before?
    if (m_isComplete)
        return;

    // Are we still parsing?
    if (m_frame->document()->parsing())
        return;

    // Still waiting for images/scripts?
    if (m_frame->document()->cachedResourceLoader()->requestCount())
        return;

    // Still waiting for elements that don't go through a FrameLoader?
    if (m_frame->document()->isDelayingLoadEvent())
        return;

    // Any frame that hasn't completed yet?
    if (!allChildrenAreComplete())
        return;

    // OK, completed.
    m_isComplete = true;
    m_frame->document()->setReadyState(Document::Complete);

    checkCallImplicitClose(); // if we didn't do it before

    completed();
    if (m_frame->page())
        checkLoadComplete();

    if (m_frame->view())
        m_frame->view()->handleLoadCompleted();
}

void FrameLoader::checkTimerFired(Timer<FrameLoader>*)
{
    RefPtr<Frame> protect(m_frame);

    if (m_shouldCallCheckCompleted)
        checkCompleted();
    if (m_shouldCallCheckLoadComplete)
        checkLoadComplete();
}

void FrameLoader::startCheckCompleteTimer()
{
    if (!(m_shouldCallCheckCompleted || m_shouldCallCheckLoadComplete))
        return;
    if (m_checkTimer.isActive())
        return;
    m_checkTimer.startOneShot(0);
}

void FrameLoader::scheduleCheckCompleted()
{
    m_shouldCallCheckCompleted = true;
    startCheckCompleteTimer();
}

void FrameLoader::scheduleCheckLoadComplete()
{
    m_shouldCallCheckLoadComplete = true;
    startCheckCompleteTimer();
}

void FrameLoader::checkCallImplicitClose()
{
    if (m_didCallImplicitClose || m_frame->document()->parsing() || m_frame->document()->isDelayingLoadEvent())
        return;

    if (!allChildrenAreComplete())
        return; // still got a frame running -> too early

    m_didCallImplicitClose = true;
    m_wasUnloadEventEmitted = false;
    m_frame->document()->implicitClose();
}

void FrameLoader::loadURLIntoChildFrame(const KURL& url, const String& referer, Frame* childFrame)
{
    ASSERT(childFrame);
    childFrame->loader()->loadURL(url, referer, "_self", false, FrameLoadTypeRedirectWithLockedBackForwardList, 0, 0);
}

ObjectContentType FrameLoader::defaultObjectContentType(const KURL& url, const String& mimeTypeIn, bool shouldPreferPlugInsForImages)
{
    return WebCore::ObjectContentNone;
}

String FrameLoader::outgoingReferrer() const
{
    // See http://www.whatwg.org/specs/web-apps/current-work/#fetching-resources
    // for why we walk the parent chain for srcdoc documents.
    Frame* frame = m_frame;
    while (frame->document()->isSrcdocDocument()) {
        frame = frame->tree()->parent();
        // Srcdoc documents cannot be top-level documents, by definition,
        // because they need to be contained in iframes with the srcdoc.
        ASSERT(frame);
    }
    return frame->loader()->m_outgoingReferrer;
}

String FrameLoader::outgoingOrigin() const
{
    return String();
}

Frame* FrameLoader::opener()
{
    return m_opener;
}

void FrameLoader::setOpener(Frame* opener)
{
    if (m_opener && !opener)
        m_client->didDisownOpener();

    if (m_opener)
        m_opener->loader()->m_openedFrames.remove(m_frame);
    if (opener)
        opener->loader()->m_openedFrames.add(m_frame);
    m_opener = opener;
}

// FIXME: This does not belong in FrameLoader!
void FrameLoader::handleFallbackContent()
{
}

void FrameLoader::provisionalLoadStarted()
{
    if (m_stateMachine.firstLayoutDone())
        m_stateMachine.advanceTo(FrameLoaderStateMachine::CommittedFirstRealLoad);
    m_client->provisionalLoadStarted();
}

void FrameLoader::resetMultipleFormSubmissionProtection()
{
    m_submittedFormURL = KURL();
}

// This does the same kind of work that didOpenURL does, except it relies on the fact
// that a higher level already checked that the URLs match and the scrolling is the right thing to do.
void FrameLoader::loadInSameDocument(const KURL& url, PassRefPtr<SerializedScriptValue> stateObject, bool isNewNavigation)
{
    // If we have a state object, we cannot also be a new navigation.
    ASSERT(!stateObject || (stateObject && !isNewNavigation));

    // Update the data source's request with the new URL to fake the URL change
    KURL oldURL = m_frame->document()->url();
    m_frame->document()->setURL(url);
    setOutgoingReferrer(url);
    documentLoader()->replaceRequestURLForSameDocumentNavigation(url);
    if (isNewNavigation && !shouldTreatURLAsSameAsCurrent(url) && !stateObject) {
        // NB: must happen after replaceRequestURLForSameDocumentNavigation(), since we add 
        // based on the current request. Must also happen before we openURL and displace the 
        // scroll position, since adding the BF item will save away scroll state.
        
        // NB2: If we were loading a long, slow doc, and the user fragment navigated before
        // it was done, currItem is now set the that slow doc, and prevItem is whatever was
        // before it.  Adding the b/f item will bump the slow doc down to prevItem, even
        // though its load is not yet done.  I think this all works out OK, for one because
        // we have already saved away the scroll and doc state for the long slow load,
        // but it's not an obvious case.
    }
    
    bool hashChange = equalIgnoringFragmentIdentifier(url, oldURL) && url.fragmentIdentifier() != oldURL.fragmentIdentifier();
    
    // If we were in the autoscroll/panScroll mode we want to stop it before following the link to the anchor
    if (hashChange)
        m_frame->eventHandler()->stopAutoscrollTimer();
    
    // It's important to model this as a load that starts and immediately finishes.
    // Otherwise, the parent frame may think we never finished loading.
    started();

    // We need to scroll to the fragment whether or not a hash change occurred, since
    // the user might have scrolled since the previous navigation.
    scrollToFragmentWithParentBoundary(url);
    
    m_isComplete = false;
    checkCompleted();

    if (isNewNavigation) {
        // This will clear previousItem from the rest of the frame tree that didn't
        // doing any loading. We need to make a pass on this now, since for fragment
        // navigation we'll not go through a real load and reach Completed state.
        checkLoadComplete();
    }

    m_client->dispatchDidNavigateWithinPage();

    m_frame->document()->statePopped(stateObject ? stateObject : SerializedScriptValue::nullValue());
    m_client->dispatchDidPopStateWithinPage();
    
    if (hashChange) {
        m_client->dispatchDidChangeLocationWithinPage();
    }
    
    // FrameLoaderClient::didFinishLoad() tells the internal load delegate the load finished with no error
    m_client->didFinishLoad();
}

bool FrameLoader::isComplete() const
{
    return m_isComplete;
}

void FrameLoader::completed()
{
    RefPtr<Frame> protect(m_frame);

    if (Frame* parent = m_frame->tree()->parent())
        parent->loader()->checkCompleted();

    if (m_frame->view())
        m_frame->view()->maintainScrollPositionAtAnchor(0);
}

void FrameLoader::started()
{
    for (Frame* frame = m_frame; frame; frame = frame->tree()->parent())
        frame->loader()->m_isComplete = false;
}

void FrameLoader::prepareForLoadStart()
{
    m_progressTracker->progressStarted();
    m_client->dispatchDidStartProvisionalLoad();

    // Notify accessibility.
    if (AXObjectCache* cache = m_frame->document()->existingAXObjectCache()) {
        AXObjectCache::AXLoadingEvent loadingEvent = loadType() == FrameLoadTypeReload ? AXObjectCache::AXLoadingReloaded : AXObjectCache::AXLoadingStarted;
        cache->frameLoadingEventNotification(m_frame, loadingEvent);
    }
}

void FrameLoader::setupForReplace()
{
    m_client->revertToProvisionalState(m_documentLoader.get());
    setState(FrameStateProvisional);
    m_provisionalDocumentLoader = m_documentLoader;
    m_documentLoader = 0;
    detachChildren();
}

void FrameLoader::loadURL(const KURL& newURL, const String& referrer, const String& frameName, bool lockHistory, FrameLoadType newLoadType,
    PassRefPtr<Event> event, PassRefPtr<FormState> prpFormState)
{
    if (m_inStopAllLoaders)
        return;

    RefPtr<FormState> formState = prpFormState;
    bool isFormSubmission = formState;
    
    ResourceRequest request(newURL);
    addExtraFieldsToRequest(request, newLoadType, true);
    if (newLoadType == FrameLoadTypeReload || newLoadType == FrameLoadTypeReloadFromOrigin)
        request.setCachePolicy(ReloadIgnoringCacheData);

    ASSERT(newLoadType != FrameLoadTypeSame);

    // The search for a target frame is done earlier in the case of form submission.
    Frame* targetFrame = isFormSubmission ? 0 : findFrameForNavigation(frameName);
    if (targetFrame && targetFrame != m_frame) {
        targetFrame->loader()->loadURL(newURL, referrer, "_self", lockHistory, newLoadType, event, formState.release());
        return;
    }

    if (m_pageDismissalEventBeingDispatched != NoDismissal)
        return;

    NavigationAction action(request, newLoadType, isFormSubmission, event);

    if (!targetFrame && !frameName.isEmpty()) {
        policyChecker()->checkNewWindowPolicy(action, FrameLoader::callContinueLoadAfterNewWindowPolicy,
            request, formState.release(), frameName, this);
        return;
    }

    RefPtr<DocumentLoader> oldDocumentLoader = m_documentLoader;

    bool sameURL = shouldTreatURLAsSameAsCurrent(newURL);
    const String& httpMethod = request.httpMethod();
    
    // Make sure to do scroll to fragment processing even if the URL is
    // exactly the same so pages with '#' links and DHTML side effects
    // work properly.
    if (shouldPerformFragmentNavigation(isFormSubmission, httpMethod, newLoadType, newURL)) {
        oldDocumentLoader->setTriggeringAction(action);
        policyChecker()->stopCheck();
        policyChecker()->setLoadType(newLoadType);
        policyChecker()->checkNavigationPolicy(request, oldDocumentLoader.get(), formState.release(),
            callContinueFragmentScrollAfterNavigationPolicy, this);
    } else {
        // must grab this now, since this load may stop the previous load and clear this flag
        bool isRedirect = m_quickRedirectComing;
        loadWithNavigationAction(request, action, lockHistory, newLoadType, formState.release());
        if (isRedirect) {
            m_quickRedirectComing = false;
            if (m_provisionalDocumentLoader)
                m_provisionalDocumentLoader->setIsClientRedirect(true);
        } else if (sameURL && newLoadType != FrameLoadTypeReload && newLoadType != FrameLoadTypeReloadFromOrigin)
            // Example of this case are sites that reload the same URL with a different cookie
            // driving the generated content, or a master frame with links that drive a target
            // frame, where the user has clicked on the same link repeatedly.
            m_loadType = FrameLoadTypeSame;
    }
}

SubstituteData FrameLoader::defaultSubstituteDataForURL(const KURL& url)
{
    if (!shouldTreatURLAsSrcdocDocument(url))
        return SubstituteData();
    String srcdoc = m_frame->ownerElement()->fastGetAttribute(srcdocAttr);
    ASSERT(!srcdoc.isNull());
    CString encodedSrcdoc = srcdoc.utf8();
    return SubstituteData(SharedBuffer::create(encodedSrcdoc.data(), encodedSrcdoc.length()), "text/html", "UTF-8", KURL());
}

void FrameLoader::load(const FrameLoadRequest& passedRequest)
{
    FrameLoadRequest request(passedRequest);

    if (m_inStopAllLoaders)
        return;

    if (!request.frameName().isEmpty()) {
        Frame* frame = findFrameForNavigation(request.frameName());
        if (frame) {
            request.setShouldCheckNewWindowPolicy(false);
            if (frame->loader() != this) {
                frame->loader()->load(request);
                return;
            }
        }
    }

    if (request.shouldCheckNewWindowPolicy()) {
        policyChecker()->checkNewWindowPolicy(NavigationAction(request.resourceRequest(), NavigationTypeOther), FrameLoader::callContinueLoadAfterNewWindowPolicy, request.resourceRequest(), 0, request.frameName(), this);
        return;
    }

    if (!request.hasSubstituteData())
        request.setSubstituteData(defaultSubstituteDataForURL(request.resourceRequest().url()));

    RefPtr<DocumentLoader> loader = m_client->createDocumentLoader(request.resourceRequest(), request.substituteData());
    if (request.lockHistory() && m_documentLoader)
        loader->setClientRedirectSourceForHistory(m_documentLoader->didCreateGlobalHistoryEntry() ? m_documentLoader->urlForHistory().string() : m_documentLoader->clientRedirectSourceForHistory());
    load(loader.get());
}

void FrameLoader::loadWithNavigationAction(const ResourceRequest& request, const NavigationAction& action, bool lockHistory, FrameLoadType type, PassRefPtr<FormState> formState)
{
    RefPtr<DocumentLoader> loader = m_client->createDocumentLoader(request, defaultSubstituteDataForURL(request.url()));
    if (lockHistory && m_documentLoader)
        loader->setClientRedirectSourceForHistory(m_documentLoader->didCreateGlobalHistoryEntry() ? m_documentLoader->urlForHistory().string() : m_documentLoader->clientRedirectSourceForHistory());

    loader->setTriggeringAction(action);
    if (m_documentLoader)
        loader->setOverrideEncoding(m_documentLoader->overrideEncoding());

    loadWithDocumentLoader(loader.get(), type, formState);
}

void FrameLoader::load(DocumentLoader* newDocumentLoader)
{
    ResourceRequest& r = newDocumentLoader->request();
    addExtraFieldsToMainResourceRequest(r);
    FrameLoadType type;

    if (shouldTreatURLAsSameAsCurrent(newDocumentLoader->originalRequest().url())) {
        r.setCachePolicy(ReloadIgnoringCacheData);
        type = FrameLoadTypeSame;
    } else if (shouldTreatURLAsSameAsCurrent(newDocumentLoader->unreachableURL()) && m_loadType == FrameLoadTypeReload)
        type = FrameLoadTypeReload;
    else
        type = FrameLoadTypeStandard;

    if (m_documentLoader)
        newDocumentLoader->setOverrideEncoding(m_documentLoader->overrideEncoding());
    
    // When we loading alternate content for an unreachable URL that we're
    // visiting in the history list, we treat it as a reload so the history list 
    // is appropriately maintained.
    //
    // FIXME: This seems like a dangerous overloading of the meaning of "FrameLoadTypeReload" ...
    // shouldn't a more explicit type of reload be defined, that means roughly 
    // "load without affecting history" ? 
    if (shouldReloadToHandleUnreachableURL(newDocumentLoader)) {
        // shouldReloadToHandleUnreachableURL() returns true only when the original load type is back-forward.
        // In this case we should save the document state now. Otherwise the state can be lost because load type is
        // changed and updateForBackForwardNavigation() will not be called when loading is committed.

        ASSERT(type == FrameLoadTypeStandard);
        type = FrameLoadTypeReload;
    }

    loadWithDocumentLoader(newDocumentLoader, type, 0);
}

void FrameLoader::loadWithDocumentLoader(DocumentLoader* loader, FrameLoadType type, PassRefPtr<FormState> prpFormState)
{
    // Retain because dispatchBeforeLoadEvent may release the last reference to it.
    RefPtr<Frame> protect(m_frame);

    ASSERT(m_client->hasWebView());

    // Unfortunately the view must be non-nil, this is ultimately due
    // to parser requiring a FrameView.  We should fix this dependency.

    ASSERT(m_frame->view());

    if (m_pageDismissalEventBeingDispatched != NoDismissal)
        return;

    if (m_frame->document())
        m_previousURL = m_frame->document()->url();

    policyChecker()->setLoadType(type);
    RefPtr<FormState> formState = prpFormState;
    bool isFormSubmission = formState;

    const KURL& newURL = loader->request().url();
    const String& httpMethod = loader->request().httpMethod();

    if (shouldPerformFragmentNavigation(isFormSubmission, httpMethod, policyChecker()->loadType(), newURL)) {
        RefPtr<DocumentLoader> oldDocumentLoader = m_documentLoader;
        NavigationAction action(loader->request(), policyChecker()->loadType(), isFormSubmission);

        oldDocumentLoader->setTriggeringAction(action);
        policyChecker()->stopCheck();
        policyChecker()->checkNavigationPolicy(loader->request(), oldDocumentLoader.get(), formState,
            callContinueFragmentScrollAfterNavigationPolicy, this);
    } else {
        if (Frame* parent = m_frame->tree()->parent())
            loader->setOverrideEncoding(parent->loader()->documentLoader()->overrideEncoding());

        policyChecker()->stopCheck();
        setPolicyDocumentLoader(loader);
        if (loader->triggeringAction().isEmpty())
            loader->setTriggeringAction(NavigationAction(loader->request(), policyChecker()->loadType(), isFormSubmission));

        if (Element* ownerElement = m_frame->ownerElement()) {
            // We skip dispatching the beforeload event if we've already
            // committed a real document load because the event would leak
            // subsequent activity by the frame which the parent frame isn't
            // supposed to learn. For example, if the child frame navigated to
            // a new URL, the parent frame shouldn't learn the URL.
            if (!m_stateMachine.committedFirstRealDocumentLoad()
                && !ownerElement->dispatchBeforeLoadEvent(loader->request().url().string())) {
                continueLoadAfterNavigationPolicy(loader->request(), formState, false);
                return;
            }
        }

        policyChecker()->checkNavigationPolicy(loader->request(), loader, formState,
            callContinueLoadAfterNavigationPolicy, this);
    }
}

void FrameLoader::reportLocalLoadFailed(Frame* frame, const String& url)
{
    ASSERT(!url.isEmpty());
    if (!frame)
        return;

    frame->document()->addConsoleMessage(SecurityMessageSource, ErrorMessageLevel, "Not allowed to load local resource: " + url);
}

const ResourceRequest& FrameLoader::initialRequest() const
{
    return activeDocumentLoader()->originalRequest();
}

bool FrameLoader::willLoadMediaElementURL(KURL& url)
{
    ResourceRequest request(url);

    unsigned long identifier;
    ResourceError error;
    requestFromDelegate(request, identifier, error);
    notifier()->sendRemainingDelegateMessages(m_documentLoader.get(), identifier, request, ResourceResponse(url, String(), -1, String(), String()), 0, -1, -1, error);

    url = request.url();

    return error.isNull();
}

bool FrameLoader::shouldReloadToHandleUnreachableURL(DocumentLoader* docLoader)
{
    return false;
}

void FrameLoader::reloadWithOverrideEncoding(const String& encoding)
{
    if (!m_documentLoader)
        return;

    ResourceRequest request = m_documentLoader->request();
    KURL unreachableURL = m_documentLoader->unreachableURL();
    if (!unreachableURL.isEmpty())
        request.setURL(unreachableURL);

    // FIXME: If the resource is a result of form submission and is not cached, the form will be silently resubmitted.
    // We should ask the user for confirmation in this case.
    request.setCachePolicy(ReturnCacheDataElseLoad);

    RefPtr<DocumentLoader> loader = m_client->createDocumentLoader(request, defaultSubstituteDataForURL(request.url()));
    setPolicyDocumentLoader(loader.get());

    loader->setOverrideEncoding(encoding);

    loadWithDocumentLoader(loader.get(), FrameLoadTypeReload, 0);
}

void FrameLoader::reloadWithOverrideURL(const KURL& overrideUrl, bool endToEndReload)
{
    if (!m_documentLoader)
        return;

    if (overrideUrl.isEmpty())
        return;

    ResourceRequest request = m_documentLoader->request();
    request.setURL(overrideUrl);
    reloadWithRequest(request, endToEndReload);
}

void FrameLoader::reload(bool endToEndReload)
{
    if (!m_documentLoader)
        return;

    // If a window is created by javascript, its main frame can have an empty but non-nil URL.
    // Reloading in this case will lose the current contents (see 4151001).
    if (m_documentLoader->request().url().isEmpty())
        return;

    // Replace error-page URL with the URL we were trying to reach.
    ResourceRequest initialRequest = m_documentLoader->request();
    KURL unreachableURL = m_documentLoader->unreachableURL();
    if (!unreachableURL.isEmpty())
        initialRequest.setURL(unreachableURL);

    reloadWithRequest(initialRequest, endToEndReload);
}

void FrameLoader::reloadWithRequest(const ResourceRequest& initialRequest, bool endToEndReload)
{
    ASSERT(m_documentLoader);

    // Create a new document loader for the reload, this will become m_documentLoader eventually,
    // but first it has to be the "policy" document loader, and then the "provisional" document loader.
    RefPtr<DocumentLoader> loader = m_client->createDocumentLoader(initialRequest, defaultSubstituteDataForURL(initialRequest.url()));

    ResourceRequest& request = loader->request();

    // FIXME: We don't have a mechanism to revalidate the main resource without reloading at the moment.
    request.setCachePolicy(ReloadIgnoringCacheData);

    // If we're about to re-post, set up action so the application can warn the user.
    if (request.httpMethod() == "POST")
        loader->setTriggeringAction(NavigationAction(request, NavigationTypeFormResubmitted));

    loader->setOverrideEncoding(m_documentLoader->overrideEncoding());
    
    loadWithDocumentLoader(loader.get(), endToEndReload ? FrameLoadTypeReloadFromOrigin : FrameLoadTypeReload, 0);
}

void FrameLoader::stopAllLoaders(ClearProvisionalItemPolicy clearProvisionalItemPolicy)
{
    ASSERT(!m_frame->document() || !m_frame->document()->inPageCache());
    if (m_pageDismissalEventBeingDispatched != NoDismissal)
        return;

    // If this method is called from within this method, infinite recursion can occur (3442218). Avoid this.
    if (m_inStopAllLoaders)
        return;
    
    // Calling stopLoading() on the provisional document loader can blow away
    // the frame from underneath.
    RefPtr<Frame> protect(m_frame);

    m_inStopAllLoaders = true;

    policyChecker()->stopCheck();

    for (RefPtr<Frame> child = m_frame->tree()->firstChild(); child; child = child->tree()->nextSibling())
        child->loader()->stopAllLoaders(clearProvisionalItemPolicy);
    if (m_provisionalDocumentLoader)
        m_provisionalDocumentLoader->stopLoading();
    if (m_documentLoader)
        m_documentLoader->stopLoading();

    setProvisionalDocumentLoader(0);

    m_checkTimer.stop();

    m_inStopAllLoaders = false;    
}

void FrameLoader::stopForUserCancel(bool deferCheckLoadComplete)
{
    stopAllLoaders();
    
    if (deferCheckLoadComplete)
        scheduleCheckLoadComplete();
    else if (m_frame->page())
        checkLoadComplete();
}

DocumentLoader* FrameLoader::activeDocumentLoader() const
{
    if (m_state == FrameStateProvisional)
        return m_provisionalDocumentLoader.get();
    return m_documentLoader.get();
}

bool FrameLoader::isLoading() const
{
    DocumentLoader* docLoader = activeDocumentLoader();
    if (!docLoader)
        return false;
    return docLoader->isLoading();
}

bool FrameLoader::frameHasLoaded() const
{
    return m_stateMachine.committedFirstRealDocumentLoad() || (m_provisionalDocumentLoader && !m_stateMachine.creatingInitialEmptyDocument()); 
}

void FrameLoader::setDocumentLoader(DocumentLoader* loader)
{
    if (!loader && !m_documentLoader)
        return;
    
    ASSERT(loader != m_documentLoader);
    ASSERT(!loader || loader->frameLoader() == this);

    m_client->prepareForDataSourceReplacement();
    detachChildren();

    // detachChildren() can trigger this frame's unload event, and therefore
    // script can run and do just about anything. For example, an unload event that calls
    // document.write("") on its parent frame can lead to a recursive detachChildren()
    // invocation for this frame. In that case, we can end up at this point with a
    // loader that hasn't been deleted but has been detached from its frame. Such a
    // DocumentLoader has been sufficiently detached that we'll end up in an inconsistent
    // state if we try to use it.
    if (loader && !loader->frame())
        return;

    if (m_documentLoader)
        m_documentLoader->detachFromFrame();

    m_documentLoader = loader;
}

void FrameLoader::setPolicyDocumentLoader(DocumentLoader* loader)
{
    if (m_policyDocumentLoader == loader)
        return;

    ASSERT(m_frame);
    if (loader)
        loader->setFrame(m_frame);
    if (m_policyDocumentLoader
            && m_policyDocumentLoader != m_provisionalDocumentLoader
            && m_policyDocumentLoader != m_documentLoader)
        m_policyDocumentLoader->detachFromFrame();

    m_policyDocumentLoader = loader;
}

void FrameLoader::setProvisionalDocumentLoader(DocumentLoader* loader)
{
    ASSERT(!loader || !m_provisionalDocumentLoader);
    ASSERT(!loader || loader->frameLoader() == this);

    if (m_provisionalDocumentLoader && m_provisionalDocumentLoader != m_documentLoader)
        m_provisionalDocumentLoader->detachFromFrame();

    m_provisionalDocumentLoader = loader;
}

double FrameLoader::timeOfLastCompletedLoad()
{
    return storedTimeOfLastCompletedLoad;
}

void FrameLoader::setState(FrameState newState)
{    
    m_state = newState;
    
    if (newState == FrameStateProvisional)
        provisionalLoadStarted();
    else if (newState == FrameStateComplete) {
        frameLoadCompleted();
        storedTimeOfLastCompletedLoad = currentTime();
        if (m_documentLoader)
            m_documentLoader->stopRecordingResponses();
    }
}

void FrameLoader::clearProvisionalLoad()
{
    setProvisionalDocumentLoader(0);
    m_progressTracker->progressCompleted();
    setState(FrameStateComplete);
}

void FrameLoader::commitProvisionalLoad()
{
    RefPtr<DocumentLoader> pdl = m_provisionalDocumentLoader;
    RefPtr<Frame> protect(m_frame);

    LOG(PageCache, "WebCoreLoading %s: About to commit provisional load from previous URL '%s' to new URL '%s'", m_frame->tree()->uniqueName().string().utf8().data(),
        m_frame->document() ? m_frame->document()->url().stringCenterEllipsizedToLength().utf8().data() : "",
        pdl ? pdl->url().stringCenterEllipsizedToLength().utf8().data() : "<no provisional DocumentLoader>");

    willTransitionToCommitted();

    if (m_loadType != FrameLoadTypeReplace)
        closeOldDataSources();

    // Call clientRedirectCancelledOrFinished() here so that the frame load delegate is notified that the redirect's
    // status has changed, if there was a redirect.  The frame load delegate may have saved some state about
    // the redirect in its -webView:willPerformClientRedirectToURL:delay:fireDate:forFrame:.  Since we are
    // just about to commit a new page, there cannot possibly be a pending redirect at this point.
    if (m_sentRedirectNotification)
        clientRedirectCancelledOrFinished(false);
    
    didOpenURL();

    LOG(Loading, "WebCoreLoading %s: Finished committing provisional load to URL %s", m_frame->tree()->uniqueName().string().utf8().data(),
        m_frame->document() ? m_frame->document()->url().stringCenterEllipsizedToLength().utf8().data() : "");

}

void FrameLoader::clientRedirectCancelledOrFinished(bool cancelWithLoadInProgress)
{
    // Note that -webView:didCancelClientRedirectForFrame: is called on the frame load delegate even if
    // the redirect succeeded.  We should either rename this API, or add a new method, like
    // -webView:didFinishClientRedirectForFrame:
    m_client->dispatchDidCancelClientRedirect();

    if (!cancelWithLoadInProgress)
        m_quickRedirectComing = false;

    m_sentRedirectNotification = false;
}

void FrameLoader::clientRedirected(const KURL& url, double seconds, double fireDate, bool lockBackForwardList)
{
    m_client->dispatchWillPerformClientRedirect(url, seconds, fireDate);
    
    // Remember that we sent a redirect notification to the frame load delegate so that when we commit
    // the next provisional load, we can send a corresponding -webView:didCancelClientRedirectForFrame:
    m_sentRedirectNotification = true;
}

bool FrameLoader::shouldReload(const KURL& currentURL, const KURL& destinationURL)
{
    // This function implements the rule: "Don't reload if navigating by fragment within
    // the same URL, but do reload if going to a new URL or to the same URL with no
    // fragment identifier at all."
    if (!destinationURL.hasFragmentIdentifier())
        return true;
    return !equalIgnoringFragmentIdentifier(currentURL, destinationURL);
}

void FrameLoader::closeOldDataSources()
{
    // FIXME: Is it important for this traversal to be postorder instead of preorder?
    // If so, add helpers for postorder traversal, and use them. If not, then lets not
    // use a recursive algorithm here.
    for (Frame* child = m_frame->tree()->firstChild(); child; child = child->tree()->nextSibling())
        child->loader()->closeOldDataSources();
    
    if (m_documentLoader)
        m_client->dispatchWillClose();

    m_client->setMainFrameDocumentReady(false); // stop giving out the actual DOMDocument to observers
}

bool FrameLoader::isHostedByObjectElement() const
{
    return false;
}

bool FrameLoader::isLoadingMainFrame() const
{
    Page* page = m_frame->page();
    return page && m_frame == page->mainFrame();
}

bool FrameLoader::isReplacing() const
{
    return m_loadType == FrameLoadTypeReplace;
}

void FrameLoader::setReplacing()
{
    m_loadType = FrameLoadTypeReplace;
}

bool FrameLoader::subframeIsLoading() const
{
    // It's most likely that the last added frame is the last to load so we walk backwards.
    for (Frame* child = m_frame->tree()->lastChild(); child; child = child->tree()->previousSibling()) {
        FrameLoader* childLoader = child->loader();
        DocumentLoader* documentLoader = childLoader->documentLoader();
        if (documentLoader && documentLoader->isLoadingInAPISense())
            return true;
        documentLoader = childLoader->provisionalDocumentLoader();
        if (documentLoader && documentLoader->isLoadingInAPISense())
            return true;
        documentLoader = childLoader->policyDocumentLoader();
        if (documentLoader)
            return true;
    }
    return false;
}

void FrameLoader::willChangeTitle(DocumentLoader* loader)
{
    m_client->willChangeTitle(loader);
}

FrameLoadType FrameLoader::loadType() const
{
    return m_loadType;
}
    
CachePolicy FrameLoader::subresourceCachePolicy() const
{
    if (m_isComplete)
        return CachePolicyVerify;

    if (m_loadType == FrameLoadTypeReloadFromOrigin)
        return CachePolicyReload;

    if (Frame* parentFrame = m_frame->tree()->parent()) {
        CachePolicy parentCachePolicy = parentFrame->loader()->subresourceCachePolicy();
        if (parentCachePolicy != CachePolicyVerify)
            return parentCachePolicy;
    }
    
    if (m_loadType == FrameLoadTypeReload)
        return CachePolicyRevalidate;

    const ResourceRequest& request(documentLoader()->request());
#if PLATFORM(MAC)
    if (request.cachePolicy() == ReloadIgnoringCacheData && !equalIgnoringCase(request.httpMethod(), "post") && ResourceRequest::useQuickLookResourceCachingQuirks())
        return CachePolicyRevalidate;
#endif

    if (request.cachePolicy() == ReturnCacheDataElseLoad)
        return CachePolicyHistoryBuffer;

    return CachePolicyVerify;
}

void FrameLoader::checkLoadCompleteForThisFrame()
{
    ASSERT(m_client->hasWebView());

    Settings* settings = m_frame->settings();

    switch (m_state) {
        case FrameStateProvisional: {
            if (m_delegateIsHandlingProvisionalLoadError)
                return;

            RefPtr<DocumentLoader> pdl = m_provisionalDocumentLoader;
            if (!pdl)
                return;
                
            // If we've received any errors we may be stuck in the provisional state and actually complete.
            const ResourceError& error = pdl->mainDocumentError();
            if (error.isNull())
                return;

            // Only reset if we aren't already going to a new provisional item.
            bool shouldReset = false;
            if (!pdl->isLoadingInAPISense() || pdl->isStopping()) {
                m_delegateIsHandlingProvisionalLoadError = true;
                m_client->dispatchDidFailProvisionalLoad(error);
                m_delegateIsHandlingProvisionalLoadError = false;

                ASSERT(!pdl->isLoading());

                // If we're in the middle of loading multipart data, we need to restore the document loader.
                if (isReplacing() && !m_documentLoader.get())
                    setDocumentLoader(m_provisionalDocumentLoader.get());

                // Finish resetting the load state, but only if another load hasn't been started by the
                // delegate callback.
                if (pdl == m_provisionalDocumentLoader)
                    clearProvisionalLoad();
                else if (activeDocumentLoader()) {
                    KURL unreachableURL = activeDocumentLoader()->unreachableURL();
                    if (!unreachableURL.isEmpty() && unreachableURL == pdl->request().url())
                        shouldReset = false;
                }
            }
            return;
        }
        
        case FrameStateCommittedPage: {
            DocumentLoader* dl = m_documentLoader.get();            
            if (!dl || (dl->isLoadingInAPISense() && !dl->isStopping()))
                return;

            setState(FrameStateComplete);

            // FIXME: Is this subsequent work important if we already navigated away?
            // Maybe there are bugs because of that, or extra work we can skip because
            // the new page is ready.

            m_client->forceLayoutForNonHTML();

            if (m_stateMachine.creatingInitialEmptyDocument() || !m_stateMachine.committedFirstRealDocumentLoad())
                return;

            if (!settings->needsDidFinishLoadOrderQuirk()) {
                m_progressTracker->progressCompleted();
                if (Page* page = m_frame->page()) {
                    if (m_frame == page->mainFrame())
                        page->resetRelevantPaintedObjectCounter();
                }
            }

            const ResourceError& error = dl->mainDocumentError();

            AXObjectCache::AXLoadingEvent loadingEvent;
            if (!error.isNull()) {
                m_client->dispatchDidFailLoad(error);
                loadingEvent = AXObjectCache::AXLoadingFailed;
            } else {
                m_client->dispatchDidFinishLoad();
                loadingEvent = AXObjectCache::AXLoadingFinished;
            }

            if (settings->needsDidFinishLoadOrderQuirk()) {
                m_progressTracker->progressCompleted();
                if (Page* page = m_frame->page()) {
                    if (m_frame == page->mainFrame())
                        page->resetRelevantPaintedObjectCounter();
                }
            }

            // Notify accessibility.
            if (AXObjectCache* cache = m_frame->document()->existingAXObjectCache())
                cache->frameLoadingEventNotification(m_frame, loadingEvent);

            return;
        }
        
        case FrameStateComplete:
            m_loadType = FrameLoadTypeStandard;
            frameLoadCompleted();
            return;
    }

    ASSERT_NOT_REACHED();
}

void FrameLoader::continueLoadAfterWillSubmitForm()
{
    if (!m_provisionalDocumentLoader)
        return;

    prepareForLoadStart();
    
    // The load might be cancelled inside of prepareForLoadStart(), nulling out the m_provisionalDocumentLoader, 
    // so we need to null check it again.
    if (!m_provisionalDocumentLoader)
        return;

    DocumentLoader* activeDocLoader = activeDocumentLoader();
    if (activeDocLoader && activeDocLoader->isLoadingMainResource())
        return;

    m_provisionalDocumentLoader->startLoadingMainResource();
}

static KURL originatingURLFromBackForwardList(Page* page)
{
    return KURL();
}

void FrameLoader::setOriginalURLForDownloadRequest(ResourceRequest& request)
{
    KURL originalURL;
    
    // If there is no referrer, assume that the download was initiated directly, so current document is
    // completely unrelated to it. See <rdar://problem/5294691>.
    // FIXME: Referrer is not sent in many other cases, so we will often miss this important information.
    // Find a better way to decide whether the download was unrelated to current document.
    if (!request.httpReferrer().isNull()) {
        // find the first item in the history that was originated by the user
        originalURL = originatingURLFromBackForwardList(m_frame->page());
    }

    if (originalURL.isNull())
        originalURL = request.url();

    if (!originalURL.protocol().isEmpty() && !originalURL.host().isEmpty()) {
        unsigned port = originalURL.port();

        // Original URL is needed to show the user where a file was downloaded from. We should make a URL that won't result in downloading the file again.
        // FIXME: Using host-only URL is a very heavy-handed approach. We should attempt to provide the actual page where the download was initiated from, as a reminder to the user.
        String hostOnlyURLString;
        if (port)
            hostOnlyURLString = makeString(originalURL.protocol(), "://", originalURL.host(), ":", String::number(port));
        else
            hostOnlyURLString = makeString(originalURL.protocol(), "://", originalURL.host());
    }
}

void FrameLoader::didLayout(LayoutMilestones milestones)
{
    m_client->dispatchDidLayout(milestones);
}

void FrameLoader::didFirstLayout()
{
    if (m_stateMachine.committedFirstRealDocumentLoad() && !m_stateMachine.isDisplayingInitialEmptyDocument() && !m_stateMachine.firstLayoutDone())
        m_stateMachine.advanceTo(FrameLoaderStateMachine::FirstLayoutDone);
}

void FrameLoader::frameLoadCompleted()
{
    // Note: Can be called multiple times.

    m_client->frameLoadCompleted();

    // After a canceled provisional load, firstLayoutDone is false.
    // Reset it to true if we're displaying a page.
    if (m_documentLoader && m_stateMachine.committedFirstRealDocumentLoad() && !m_stateMachine.isDisplayingInitialEmptyDocument() && !m_stateMachine.firstLayoutDone())
        m_stateMachine.advanceTo(FrameLoaderStateMachine::FirstLayoutDone);
}

void FrameLoader::detachChildren()
{
    typedef Vector<RefPtr<Frame> > FrameVector;
    FrameVector childrenToDetach;
    childrenToDetach.reserveCapacity(m_frame->tree()->childCount());
    for (Frame* child = m_frame->tree()->lastChild(); child; child = child->tree()->previousSibling())
        childrenToDetach.append(child);
    FrameVector::iterator end = childrenToDetach.end();
    for (FrameVector::iterator it = childrenToDetach.begin(); it != end; ++it)
        (*it)->loader()->detachFromParent();
}

void FrameLoader::closeAndRemoveChild(Frame* child)
{
    child->tree()->detachFromParent();

    child->setView(0);
    if (child->ownerElement() && child->page())
        child->page()->decrementSubframeCount();
    child->willDetachPage();
    child->detachFromPage();

    m_frame->tree()->removeChild(child);
}

// Called every time a resource is completely loaded or an error is received.
void FrameLoader::checkLoadComplete()
{
    ASSERT(m_client->hasWebView());
    
    m_shouldCallCheckLoadComplete = false;

    // FIXME: Always traversing the entire frame tree is a bit inefficient, but 
    // is currently needed in order to null out the previous history item for all frames.
    if (Page* page = m_frame->page()) {
        Vector<RefPtr<Frame>, 10> frames;
        for (RefPtr<Frame> frame = page->mainFrame(); frame; frame = frame->tree()->traverseNext())
            frames.append(frame);
        // To process children before their parents, iterate the vector backwards.
        for (size_t i = frames.size(); i; --i)
            frames[i - 1]->loader()->checkLoadCompleteForThisFrame();
    }
}

int FrameLoader::numPendingOrLoadingRequests(bool recurse) const
{
    if (!recurse)
        return m_frame->document()->cachedResourceLoader()->requestCount();

    int count = 0;
    for (Frame* frame = m_frame; frame; frame = frame->tree()->traverseNext(m_frame))
        count += frame->document()->cachedResourceLoader()->requestCount();
    return count;
}

String FrameLoader::userAgent(const KURL& url) const
{
    String userAgent = m_client->userAgent(url);
    return userAgent;
}

void FrameLoader::handledOnloadEvents()
{
    m_client->dispatchDidHandleOnloadEvents();

    if (documentLoader())
        documentLoader()->handledOnloadEvents();
}

void FrameLoader::frameDetached()
{
    stopAllLoaders();
    m_frame->document()->stopActiveDOMObjects();
    detachFromParent();
}

void FrameLoader::detachFromParent()
{
    RefPtr<Frame> protect(m_frame);

    closeURL();
    detachChildren();
    // stopAllLoaders() needs to be called after detachChildren(), because detachedChildren()
    // will trigger the unload event handlers of any child frames, and those event
    // handlers might start a new subresource load in this frame.
    stopAllLoaders();

    detachViewsAndDocumentLoader();

    m_progressTracker.clear();

    if (Frame* parent = m_frame->tree()->parent()) {
        parent->loader()->closeAndRemoveChild(m_frame);
        parent->loader()->scheduleCheckCompleted();
    } else {
        m_frame->setView(0);
        m_frame->willDetachPage();
        m_frame->detachFromPage();
    }
}

void FrameLoader::detachViewsAndDocumentLoader()
{
    m_client->detachedFromParent2();
    setDocumentLoader(0);
    m_client->detachedFromParent3();
}
    
void FrameLoader::addExtraFieldsToSubresourceRequest(ResourceRequest& request)
{
    addExtraFieldsToRequest(request, m_loadType, false);
}

void FrameLoader::addExtraFieldsToMainResourceRequest(ResourceRequest& request)
{
    // FIXME: Using m_loadType seems wrong for some callers.
    // If we are only preparing to load the main resource, that is previous load's load type!
    addExtraFieldsToRequest(request, m_loadType, true);
}

void FrameLoader::addExtraFieldsToRequest(ResourceRequest& request, FrameLoadType loadType, bool mainResource)
{
    // The remaining modifications are only necessary for HTTP and HTTPS.
    if (!request.url().isEmpty() && !request.url().protocolIsInHTTPFamily())
        return;

    applyUserAgent(request);

    if (!mainResource) {
        if (request.isConditional())
            request.setCachePolicy(ReloadIgnoringCacheData);
        else if (documentLoader()->isLoadingInAPISense()) {
            // If we inherit cache policy from a main resource, we use the DocumentLoader's
            // original request cache policy for two reasons:
            // 1. For POST requests, we mutate the cache policy for the main resource,
            //    but we do not want this to apply to subresources
            // 2. Delegates that modify the cache policy using willSendRequest: should
            //    not affect any other resources. Such changes need to be done
            //    per request.
            ResourceRequestCachePolicy mainDocumentOriginalCachePolicy = documentLoader()->originalRequest().cachePolicy();
            // Back-forward navigations try to load main resource from cache only to avoid re-submitting form data, and start over (with a warning dialog) if that fails.
            // This policy is set on initial request too, but should not be inherited.
            ResourceRequestCachePolicy subresourceCachePolicy = (mainDocumentOriginalCachePolicy == ReturnCacheDataDontLoad) ? ReturnCacheDataElseLoad : mainDocumentOriginalCachePolicy;
            request.setCachePolicy(subresourceCachePolicy);
        } else
            request.setCachePolicy(UseProtocolCachePolicy);

    // FIXME: Other FrameLoader functions have duplicated code for setting cache policy of main request when reloading.
    // It seems better to manage it explicitly than to hide the logic inside addExtraFieldsToRequest().
    } else if (loadType == FrameLoadTypeReload || loadType == FrameLoadTypeReloadFromOrigin || request.isConditional())
        request.setCachePolicy(ReloadIgnoringCacheData);

    if (request.cachePolicy() == ReloadIgnoringCacheData) {
        if (loadType == FrameLoadTypeReload)
            request.setHTTPHeaderField("Cache-Control", "max-age=0");
        else if (loadType == FrameLoadTypeReloadFromOrigin) {
            request.setHTTPHeaderField("Cache-Control", "no-cache");
            request.setHTTPHeaderField("Pragma", "no-cache");
        }
    }
    
    if (mainResource)
        request.setHTTPAccept(defaultAcceptHeader);

    // Make sure we send the Origin header.
    addHTTPOriginIfNeeded(request, String());

    // Only set fallback array if it's still empty (later attempts may be incorrect, see bug 117818).
    if (request.responseContentDispositionEncodingFallbackArray().isEmpty()) {
        // Always try UTF-8. If that fails, try frame encoding (if any) and then the default.
        Settings* settings = m_frame->settings();
        request.setResponseContentDispositionEncodingFallbackArray("UTF-8", m_frame->document()->encoding(), settings ? settings->defaultTextEncodingName() : String());
    }
}

void FrameLoader::addHTTPOriginIfNeeded(ResourceRequest& request, const String& origin)
{
}

void FrameLoader::loadPostRequest(const ResourceRequest& inRequest, const String& referrer, const String& frameName, bool lockHistory, FrameLoadType loadType, PassRefPtr<Event> event, PassRefPtr<FormState> prpFormState)
{
    RefPtr<FormState> formState = prpFormState;

    // Previously when this method was reached, the original FrameLoadRequest had been deconstructed to build a 
    // bunch of parameters that would come in here and then be built back up to a ResourceRequest.  In case
    // any caller depends on the immutability of the original ResourceRequest, I'm rebuilding a ResourceRequest
    // from scratch as it did all along.
    const KURL& url = inRequest.url();
    RefPtr<FormData> formData = inRequest.httpBody();
    const String& contentType = inRequest.httpContentType();
    String origin = inRequest.httpOrigin();

    ResourceRequest workingResourceRequest(url);    

    if (!referrer.isEmpty())
        workingResourceRequest.setHTTPReferrer(referrer);
    workingResourceRequest.setHTTPOrigin(origin);
    workingResourceRequest.setHTTPMethod("POST");
    workingResourceRequest.setHTTPBody(formData);
    workingResourceRequest.setHTTPContentType(contentType);
    addExtraFieldsToRequest(workingResourceRequest, loadType, true);

    NavigationAction action(workingResourceRequest, loadType, true, event);

    if (!frameName.isEmpty()) {
        // The search for a target frame is done earlier in the case of form submission.
        if (Frame* targetFrame = formState ? 0 : findFrameForNavigation(frameName))
            targetFrame->loader()->loadWithNavigationAction(workingResourceRequest, action, lockHistory, loadType, formState.release());
        else
            policyChecker()->checkNewWindowPolicy(action, FrameLoader::callContinueLoadAfterNewWindowPolicy, workingResourceRequest, formState.release(), frameName, this);
    } else {
        // must grab this now, since this load may stop the previous load and clear this flag
        bool isRedirect = m_quickRedirectComing;
        loadWithNavigationAction(workingResourceRequest, action, lockHistory, loadType, formState.release());    
        if (isRedirect) {
            m_quickRedirectComing = false;
            if (m_provisionalDocumentLoader)
                m_provisionalDocumentLoader->setIsClientRedirect(true);
        }
    }
}

unsigned long FrameLoader::loadResourceSynchronously(const ResourceRequest& request, StoredCredentials storedCredentials, ClientCredentialPolicy clientCredentialPolicy, ResourceError& error, ResourceResponse& response, Vector<char>& data)
{
    ASSERT(m_frame->document());

    ResourceRequest initialRequest = request;
    initialRequest.setTimeoutInterval(10);
    
    addHTTPOriginIfNeeded(initialRequest, outgoingOrigin());

    addExtraFieldsToSubresourceRequest(initialRequest);

    unsigned long identifier = 0;    
    ResourceRequest newRequest(initialRequest);
    requestFromDelegate(newRequest, identifier, error);

    if (error.isNull()) {
        ASSERT(!newRequest.isNull());
    }
    notifier()->sendRemainingDelegateMessages(m_documentLoader.get(), identifier, request, response, data.data(), data.size(), -1, error);
    return identifier;
}

const ResourceRequest& FrameLoader::originalRequest() const
{
    return activeDocumentLoader()->originalRequestCopy();
}

void FrameLoader::receivedMainResourceError(const ResourceError& error)
{
    // Retain because the stop may release the last reference to it.
    RefPtr<Frame> protect(m_frame);

    RefPtr<DocumentLoader> loader = activeDocumentLoader();
    // FIXME: Don't want to do this if an entirely new load is going, so should check
    // that both data sources on the frame are either this or nil.
    stop();
    if (m_client->shouldFallBack(error))
        handleFallbackContent();

    if (m_state == FrameStateProvisional && m_provisionalDocumentLoader) {
        if (m_submittedFormURL == m_provisionalDocumentLoader->originalRequestCopy().url())
            m_submittedFormURL = KURL();
            
        // Call clientRedirectCancelledOrFinished here so that the frame load delegate is notified that the redirect's
        // status has changed, if there was a redirect. The frame load delegate may have saved some state about
        // the redirect in its -webView:willPerformClientRedirectToURL:delay:fireDate:forFrame:. Since we are definitely
        // not going to use this provisional resource, as it was cancelled, notify the frame load delegate that the redirect
        // has ended.
        if (m_sentRedirectNotification)
            clientRedirectCancelledOrFinished(false);
    }

    checkCompleted();
    if (m_frame->page())
        checkLoadComplete();
}

void FrameLoader::callContinueFragmentScrollAfterNavigationPolicy(void* argument,
    const ResourceRequest& request, PassRefPtr<FormState>, bool shouldContinue)
{
    FrameLoader* loader = static_cast<FrameLoader*>(argument);
    loader->continueFragmentScrollAfterNavigationPolicy(request, shouldContinue);
}

void FrameLoader::continueFragmentScrollAfterNavigationPolicy(const ResourceRequest& request, bool shouldContinue)
{
    m_quickRedirectComing = false;

    if (!shouldContinue)
        return;

    // If we have a provisional request for a different document, a fragment scroll should cancel it.
    if (m_provisionalDocumentLoader && !equalIgnoringFragmentIdentifier(m_provisionalDocumentLoader->request().url(), request.url())) {
        m_provisionalDocumentLoader->stopLoading();
        setProvisionalDocumentLoader(0);
    }

    bool isRedirect = m_quickRedirectComing || policyChecker()->loadType() == FrameLoadTypeRedirectWithLockedBackForwardList;    
    loadInSameDocument(request.url(), 0, !isRedirect);
}

bool FrameLoader::shouldPerformFragmentNavigation(bool isFormSubmission, const String& httpMethod, FrameLoadType loadType, const KURL& url)
{
    // We don't do this if we are submitting a form with method other than "GET", explicitly reloading,
    // currently displaying a frameset, or if the URL does not have a fragment.
    // These rules were originally based on what KHTML was doing in KHTMLPart::openURL.

    // FIXME: What about load types other than Standard and Reload?

    return (!isFormSubmission || equalIgnoringCase(httpMethod, "GET"))
        && loadType != FrameLoadTypeReload
        && loadType != FrameLoadTypeReloadFromOrigin
        && loadType != FrameLoadTypeSame
        && !shouldReload(m_frame->document()->url(), url)
        // We don't want to just scroll if a link from within a
        // frameset is trying to reload the frameset into _top.
        && !m_frame->document()->isFrameSet();
}

void FrameLoader::scrollToFragmentWithParentBoundary(const KURL& url)
{
    FrameView* view = m_frame->view();
    if (!view)
        return;

    view->scrollToFragment(url);
}

void FrameLoader::callContinueLoadAfterNavigationPolicy(void* argument,
    const ResourceRequest& request, PassRefPtr<FormState> formState, bool shouldContinue)
{
    FrameLoader* loader = static_cast<FrameLoader*>(argument);
    loader->continueLoadAfterNavigationPolicy(request, formState, shouldContinue);
}

bool FrameLoader::shouldClose()
{
    return true;
}

void FrameLoader::continueLoadAfterNavigationPolicy(const ResourceRequest&, PassRefPtr<FormState> formState, bool shouldContinue)
{
    // If we loaded an alternate page to replace an unreachableURL, we'll get in here with a
    // nil policyDataSource because loading the alternate page will have passed
    // through this method already, nested; otherwise, policyDataSource should still be set.
    ASSERT(m_policyDocumentLoader || !m_provisionalDocumentLoader->unreachableURL().isEmpty());

    bool isTargetItem = false;

    // Two reasons we can't continue:
    //    1) Navigation policy delegate said we can't so request is nil. A primary case of this 
    //       is the user responding Cancel to the form repost nag sheet.
    //    2) User responded Cancel to an alert popped up by the before unload event handler.
    bool canContinue = shouldContinue && shouldClose();

    if (!canContinue) {
        // If we were waiting for a quick redirect, but the policy delegate decided to ignore it, then we 
        // need to report that the client redirect was cancelled.
        if (m_quickRedirectComing)
            clientRedirectCancelledOrFinished(false);

        setPolicyDocumentLoader(0);
        return;
    }

    FrameLoadType type = policyChecker()->loadType();
    // A new navigation is in progress, so don't clear the history's provisional item.
    stopAllLoaders(ShouldNotClearProvisionalItem);
    
    // <rdar://problem/6250856> - In certain circumstances on pages with multiple frames, stopAllLoaders()
    // might detach the current FrameLoader, in which case we should bail on this newly defunct load. 
    if (!m_frame->page())
        return;

    setProvisionalDocumentLoader(m_policyDocumentLoader.get());
    m_loadType = type;
    setState(FrameStateProvisional);

    setPolicyDocumentLoader(0);

    if (formState)
        m_client->dispatchWillSubmitForm(&PolicyChecker::continueLoadAfterWillSubmitForm, formState);
    else
        continueLoadAfterWillSubmitForm();
}

void FrameLoader::callContinueLoadAfterNewWindowPolicy(void* argument,
    const ResourceRequest& request, PassRefPtr<FormState> formState, const String& frameName, const NavigationAction& action, bool shouldContinue)
{
    FrameLoader* loader = static_cast<FrameLoader*>(argument);
    loader->continueLoadAfterNewWindowPolicy(request, formState, frameName, action, shouldContinue);
}

void FrameLoader::continueLoadAfterNewWindowPolicy(const ResourceRequest& request,
    PassRefPtr<FormState> formState, const String& frameName, const NavigationAction& action, bool shouldContinue)
{
    if (!shouldContinue)
        return;

    RefPtr<Frame> frame = m_frame;
    RefPtr<Frame> mainFrame = m_client->dispatchCreatePage(action);
    if (!mainFrame)
        return;

    if (frameName != "_blank")
        mainFrame->tree()->setName(frameName);

    mainFrame->page()->setOpenedByDOM();
    mainFrame->loader()->m_client->dispatchShow();
    if (!m_suppressOpenerInNewFrame) {
        mainFrame->loader()->setOpener(frame.get());
        mainFrame->document()->setReferrerPolicy(frame->document()->referrerPolicy());
    }
    mainFrame->loader()->loadWithNavigationAction(request, NavigationAction(request), false, FrameLoadTypeStandard, formState);
}

void FrameLoader::requestFromDelegate(ResourceRequest& request, unsigned long& identifier, ResourceError& error)
{
    ASSERT(!request.isNull());

    identifier = 0;
    if (Page* page = m_frame->page()) {
        identifier = page->progress()->createUniqueIdentifier();
        notifier()->assignIdentifierToInitialRequest(identifier, m_documentLoader.get(), request);
    }

    ResourceRequest newRequest(request);
    notifier()->dispatchWillSendRequest(m_documentLoader.get(), identifier, newRequest, ResourceResponse());

    if (newRequest.isNull())
        error = cancelledError(request);
    else
        error = ResourceError();

    request = newRequest;
}

void FrameLoader::loadedResourceFromMemoryCache(CachedResource* resource, ResourceRequest& newRequest)
{
    newRequest = ResourceRequest(resource->url());

    Page* page = m_frame->page();
    if (!page)
        return;

    if (!resource->shouldSendResourceLoadCallbacks() || m_documentLoader->haveToldClientAboutLoad(resource->url()))
        return;

    // Main resource delegate messages are synthesized in MainResourceLoader, so we must not send them here.
    if (resource->type() == CachedResource::MainResource)
        return;

    if (!page->areMemoryCacheClientCallsEnabled()) {
        m_documentLoader->recordMemoryCacheLoadForFutureClientNotification(resource->resourceRequest());
        m_documentLoader->didTellClientAboutLoad(resource->url());
        return;
    }

    if (m_client->dispatchDidLoadResourceFromMemoryCache(m_documentLoader.get(), newRequest, resource->response(), resource->encodedSize())) {
        m_documentLoader->didTellClientAboutLoad(resource->url());
        return;
    }

    unsigned long identifier;
    ResourceError error;
    requestFromDelegate(newRequest, identifier, error);
    notifier()->sendRemainingDelegateMessages(m_documentLoader.get(), identifier, newRequest, resource->response(), 0, resource->encodedSize(), 0, error);
}

void FrameLoader::applyUserAgent(ResourceRequest& request)
{
    String userAgent = this->userAgent(request.url());
    ASSERT(!userAgent.isNull());
    request.setHTTPUserAgent(userAgent);
}

bool FrameLoader::shouldInterruptLoadForXFrameOptions(const String& content, const KURL& url, unsigned long requestIdentifier)
{
    return false;
}

bool FrameLoader::shouldTreatURLAsSameAsCurrent(const KURL& url) const
{
    return false;
}

bool FrameLoader::shouldTreatURLAsSrcdocDocument(const KURL& url) const
{
    if (!equalIgnoringCase(url.string(), "about:srcdoc"))
        return false;
    HTMLFrameOwnerElement* ownerElement = m_frame->ownerElement();
    if (!ownerElement)
        return false;
    if (!ownerElement->hasTagName(iframeTag))
        return false;
    return ownerElement->fastHasAttribute(srcdocAttr);
}

void FrameLoader::checkDidPerformFirstNavigation()
{
    Page* page = m_frame->page();
    if (!page)
        return;

    if (!m_didPerformFirstNavigation) {
        m_didPerformFirstNavigation = true;
        m_client->didPerformFirstNavigation();
    }
}

Frame* FrameLoader::findFrameForNavigation(const AtomicString& name, Document* activeDocument)
{
    Frame* frame = m_frame->tree()->find(name);

    // From http://www.whatwg.org/specs/web-apps/current-work/#seamlessLinks:
    //
    // If the source browsing context is the same as the browsing context
    // being navigated, and this browsing context has its seamless browsing
    // context flag set, and the browsing context being navigated was not
    // chosen using an explicit self-navigation override, then find the
    // nearest ancestor browsing context that does not have its seamless
    // browsing context flag set, and continue these steps as if that
    // browsing context was the one that was going to be navigated instead.
    if (frame == m_frame && name != "_self" && m_frame->document()->shouldDisplaySeamlesslyWithParent()) {
        for (Frame* ancestor = m_frame; ancestor; ancestor = ancestor->tree()->parent()) {
            if (!ancestor->document()->shouldDisplaySeamlesslyWithParent()) {
                frame = ancestor;
                break;
            }
        }
        ASSERT(frame != m_frame);
    }

    if (activeDocument) {
        if (!activeDocument->canNavigate(frame))
            return 0;
    } else {
        // FIXME: Eventually all callers should supply the actual activeDocument
        // so we can call canNavigate with the right document.
        if (!m_frame->document()->canNavigate(frame))
            return 0;
    }

    return frame;
}


ResourceError FrameLoader::cancelledError(const ResourceRequest& request) const
{
    ResourceError error = m_client->cancelledError(request);
    error.setIsCancellation(true);
    return error;
}

void FrameLoader::setTitle(const StringWithDirection& title)
{
    documentLoader()->setTitle(title);
}

String FrameLoader::referrer() const
{
    return m_documentLoader ? m_documentLoader->request().httpReferrer() : "";
}

void FrameLoader::dispatchDocumentElementAvailable()
{
    m_client->documentElementAvailable();
}

void FrameLoader::dispatchDidClearWindowObjectsInAllWorlds()
{
    return;
}

void FrameLoader::dispatchDidClearWindowObjectInWorld(DOMWrapperWorld* world)
{
    return;
}

void FrameLoader::dispatchGlobalObjectAvailableInAllWorlds()
{
}

void FrameLoader::didChangeTitle(DocumentLoader* loader)
{
    m_client->didChangeTitle(loader);

    if (loader == m_documentLoader) {
        // This must go through the WebFrame because it has the right notion of the current b/f item.
        m_client->setTitle(loader->title(), loader->urlForHistory());
        m_client->setMainFrameDocumentReady(true); // update observers with new DOMDocument
        m_client->dispatchDidReceiveTitle(loader->title());
    }
}

void FrameLoader::dispatchDidCommitLoad()
{
    if (m_stateMachine.creatingInitialEmptyDocument())
        return;

    m_client->dispatchDidCommitLoad();

    if (isLoadingMainFrame()) {
        m_frame->page()->resetSeenMediaEngines();
    }

    if (m_frame->page()->mainFrame() == m_frame)
        m_frame->page()->featureObserver()->didCommitLoad();

}

void FrameLoader::tellClientAboutPastMemoryCacheLoads()
{
    ASSERT(m_frame->page());
    ASSERT(m_frame->page()->areMemoryCacheClientCallsEnabled());

    if (!m_documentLoader)
        return;

    Vector<ResourceRequest> pastLoads;
    m_documentLoader->takeMemoryCacheLoadsForClientNotification(pastLoads);

    size_t size = pastLoads.size();
    for (size_t i = 0; i < size; ++i) {
        CachedResource* resource = memoryCache()->resourceForRequest(pastLoads[i]);

        // FIXME: These loads, loaded from cache, but now gone from the cache by the time
        // Page::setMemoryCacheClientCallsEnabled(true) is called, will not be seen by the client.
        // Consider if there's some efficient way of remembering enough to deliver this client call.
        // We have the URL, but not the rest of the response or the length.
        if (!resource)
            continue;

        ResourceRequest request(resource->url());
        m_client->dispatchDidLoadResourceFromMemoryCache(m_documentLoader.get(), request, resource->response(), resource->encodedSize());
    }
}

NetworkingContext* FrameLoader::networkingContext() const
{
    return m_networkingContext.get();
}

void FrameLoader::loadProgressingStatusChanged()
{
    FrameView* view = m_frame->page()->mainFrame()->view();
    view->updateLayerFlushThrottlingInAllFrames();
    view->adjustTiledBackingCoverage();
}

void FrameLoader::forcePageTransitionIfNeeded()
{
    m_client->forcePageTransitionIfNeeded();
}

bool FrameLoaderClient::hasHTMLView() const
{
    return true;
}

} // namespace WebCore
