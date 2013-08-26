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
 
#ifndef TextControlInnerElements_h
#define TextControlInnerElements_h

#include "html/HTMLDivElement.h"
#include <wtf/Forward.h>

namespace WebCore {

class TextControlInnerContainer FINAL : public HTMLDivElement {
public:
    static PassRefPtr<TextControlInnerContainer> create(Document*);
protected:
    TextControlInnerContainer(Document*);
    virtual RenderObject* createRenderer(RenderArena*, RenderStyle*);
};

class TextControlInnerElement FINAL : public HTMLDivElement {
public:
    static PassRefPtr<TextControlInnerElement> create(Document*);

protected:
    TextControlInnerElement(Document*);
    virtual PassRefPtr<RenderStyle> customStyleForRenderer() OVERRIDE;

private:
    virtual bool isMouseFocusable() const OVERRIDE { return false; }
};

class TextControlInnerTextElement FINAL : public HTMLDivElement {
public:
    static PassRefPtr<TextControlInnerTextElement> create(Document*);

    virtual void defaultEventHandler(Event*);

private:
    TextControlInnerTextElement(Document*);
    virtual RenderObject* createRenderer(RenderArena*, RenderStyle*);
    virtual PassRefPtr<RenderStyle> customStyleForRenderer() OVERRIDE;
    virtual bool isMouseFocusable() const OVERRIDE { return false; }
};

class SearchFieldResultsButtonElement FINAL : public HTMLDivElement {
public:
    static PassRefPtr<SearchFieldResultsButtonElement> create(Document*);

    virtual void defaultEventHandler(Event*);
    virtual bool willRespondToMouseClickEvents() OVERRIDE;

private:
    SearchFieldResultsButtonElement(Document*);
    virtual const AtomicString& shadowPseudoId() const;
    virtual bool isMouseFocusable() const OVERRIDE { return false; }
};

class SearchFieldCancelButtonElement FINAL : public HTMLDivElement {
public:
    static PassRefPtr<SearchFieldCancelButtonElement> create(Document*);

    virtual void defaultEventHandler(Event*);
    virtual bool willRespondToMouseClickEvents() OVERRIDE;

private:
    SearchFieldCancelButtonElement(Document*);
    virtual const AtomicString& shadowPseudoId() const;
    virtual void detach(const AttachContext& = AttachContext()) OVERRIDE;
    virtual bool isMouseFocusable() const OVERRIDE { return false; }

    bool m_capturing;
};

} // namespace

#endif
