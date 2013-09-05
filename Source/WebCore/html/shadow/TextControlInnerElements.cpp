/*
 * Copyright (C) 2006, 2008, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "html/shadow/TextControlInnerElements.h"

#include "dom/Document.h"
#include "page/EventHandler.h"
#include "dom/EventNames.h"
#include "page/Frame.h"
#include "html/HTMLInputElement.h"
#include "HTMLNames.h"
#include "dom/MouseEvent.h"
#include "rendering/RenderSearchField.h"
#include "rendering/RenderTextControl.h"
#include "rendering/RenderView.h"
#include "bindings/dui/ScriptController.h"
#include "dom/TextEvent.h"
#include "dom/TextEventInputType.h"

namespace WebCore {

using namespace HTMLNames;

TextControlInnerContainer::TextControlInnerContainer(Document* document)
    : HTMLDivElement(divTag, document)
{
}

PassRefPtr<TextControlInnerContainer> TextControlInnerContainer::create(Document* document)
{
    return adoptRef(new TextControlInnerContainer(document));
}
    
RenderObject* TextControlInnerContainer::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderTextControlInnerContainer(this);
}

TextControlInnerElement::TextControlInnerElement(Document* document)
    : HTMLDivElement(divTag, document)
{
    setHasCustomStyleCallbacks();
}

PassRefPtr<TextControlInnerElement> TextControlInnerElement::create(Document* document)
{
    return adoptRef(new TextControlInnerElement(document));
}

PassRefPtr<RenderStyle> TextControlInnerElement::customStyleForRenderer()
{
    RenderTextControlSingleLine* parentRenderer = toRenderTextControlSingleLine(shadowHost()->renderer());
    return parentRenderer->createInnerBlockStyle(parentRenderer->style());
}

// ---------------------------

inline TextControlInnerTextElement::TextControlInnerTextElement(Document* document)
    : HTMLDivElement(divTag, document)
{
    setHasCustomStyleCallbacks();
}

PassRefPtr<TextControlInnerTextElement> TextControlInnerTextElement::create(Document* document)
{
    return adoptRef(new TextControlInnerTextElement(document));
}

void TextControlInnerTextElement::defaultEventHandler(Event* event)
{
    // FIXME: In the future, we should add a way to have default event listeners.
    // Then we would add one to the text field's inner div, and we wouldn't need this subclass.
    // Or possibly we could just use a normal event listener.
    if (event->isBeforeTextInsertedEvent() || event->type() == eventNames().webkitEditableContentChangedEvent) {
        Element* shadowAncestor = shadowHost();
        // A TextControlInnerTextElement can have no host if its been detached,
        // but kept alive by an EditCommand. In this case, an undo/redo can
        // cause events to be sent to the TextControlInnerTextElement. To
        // prevent an infinite loop, we must check for this case before sending
        // the event up the chain.
        if (shadowAncestor)
            shadowAncestor->defaultEventHandler(event);
    }
    if (!event->defaultHandled())
        HTMLDivElement::defaultEventHandler(event);
}

RenderObject* TextControlInnerTextElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderTextControlInnerBlock(this);
}

PassRefPtr<RenderStyle> TextControlInnerTextElement::customStyleForRenderer()
{
    RenderTextControl* parentRenderer = toRenderTextControl(shadowHost()->renderer());
    return parentRenderer->createInnerTextStyle(parentRenderer->style());
}

// ----------------------------

inline SearchFieldResultsButtonElement::SearchFieldResultsButtonElement(Document* document)
    : HTMLDivElement(divTag, document)
{
}

PassRefPtr<SearchFieldResultsButtonElement> SearchFieldResultsButtonElement::create(Document* document)
{
    return adoptRef(new SearchFieldResultsButtonElement(document));
}

const AtomicString& SearchFieldResultsButtonElement::shadowPseudoId() const
{
    DEFINE_STATIC_LOCAL(AtomicString, resultsId, ("-webkit-search-results-button", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, resultsDecorationId, ("-webkit-search-results-decoration", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, decorationId, ("-webkit-search-decoration", AtomicString::ConstructFromLiteral));
    Element* host = shadowHost();
    if (!host)
        return resultsId;
    if (HTMLInputElement* input = host->toInputElement()) {
        if (input->maxResults() < 0)
            return decorationId;
        if (input->maxResults() > 0)
            return resultsId;
        return resultsDecorationId;
    }
    return resultsId;
}

void SearchFieldResultsButtonElement::defaultEventHandler(Event* event)
{
    // On mousedown, bring up a menu, if needed
    HTMLInputElement* input = toHTMLInputElement(shadowHost());
    if (input && event->type() == eventNames().mousedownEvent && event->isMouseEvent() && static_cast<MouseEvent*>(event)->button() == LeftButton) {
        input->focus();
        input->select();
        RenderSearchField* renderer = toRenderSearchField(input->renderer());
        if (renderer->popupIsVisible())
            renderer->hidePopup();
        else if (input->maxResults() > 0)
            renderer->showPopup();
        event->setDefaultHandled();
    }

    if (!event->defaultHandled())
        HTMLDivElement::defaultEventHandler(event);
}

bool SearchFieldResultsButtonElement::willRespondToMouseClickEvents()
{
    return true;
}

// ----------------------------

inline SearchFieldCancelButtonElement::SearchFieldCancelButtonElement(Document* document)
    : HTMLDivElement(divTag, document)
    , m_capturing(false)
{
}

PassRefPtr<SearchFieldCancelButtonElement> SearchFieldCancelButtonElement::create(Document* document)
{
    return adoptRef(new SearchFieldCancelButtonElement(document));
}

const AtomicString& SearchFieldCancelButtonElement::shadowPseudoId() const
{
    DEFINE_STATIC_LOCAL(AtomicString, pseudoId, ("-webkit-search-cancel-button", AtomicString::ConstructFromLiteral));
    return pseudoId;
}

void SearchFieldCancelButtonElement::detach(const AttachContext& context)
{
    if (m_capturing) {
        if (Frame* frame = document()->frame())
            frame->eventHandler()->setCapturingMouseEventsNode(0);
    }
    HTMLDivElement::detach(context);
}


void SearchFieldCancelButtonElement::defaultEventHandler(Event* event)
{
    // If the element is visible, on mouseup, clear the value, and set selection
    RefPtr<HTMLInputElement> input(toHTMLInputElement(shadowHost()));
    if (!input || input->isDisabledOrReadOnly()) {
        if (!event->defaultHandled())
            HTMLDivElement::defaultEventHandler(event);
        return;
    }

    if (event->type() == eventNames().mousedownEvent && event->isMouseEvent() && static_cast<MouseEvent*>(event)->button() == LeftButton) {
        if (renderer() && renderer()->visibleToHitTesting()) {
            if (Frame* frame = document()->frame()) {
                frame->eventHandler()->setCapturingMouseEventsNode(this);
                m_capturing = true;
            }
        }
        input->focus();
        input->select();
        event->setDefaultHandled();
    }
    if (event->type() == eventNames().mouseupEvent && event->isMouseEvent() && static_cast<MouseEvent*>(event)->button() == LeftButton) {
        if (m_capturing) {
            if (Frame* frame = document()->frame()) {
                frame->eventHandler()->setCapturingMouseEventsNode(0);
                m_capturing = false;
            }
            if (hovered()) {
                String oldValue = input->value();
                input->setValueForUser("");
                input->onSearch();
                event->setDefaultHandled();
            }
        }
    }

    if (!event->defaultHandled())
        HTMLDivElement::defaultEventHandler(event);
}

bool SearchFieldCancelButtonElement::willRespondToMouseClickEvents()
{
    const HTMLInputElement* input = toHTMLInputElement(shadowHost());
    if (input && !input->isDisabledOrReadOnly())
        return true;

    return HTMLDivElement::willRespondToMouseClickEvents();
}

}
