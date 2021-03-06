/*
 * Copyright (C) 2006, 2007, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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

#ifndef DOMWindow_h
#define DOMWindow_h

#include "dom/ContextDestructionObserver.h"
#include "dom/EventTarget.h"
#include "page/FrameDestructionObserver.h"
#include "platform/KURL.h"
#include "platform/Supplementable.h"

namespace WebCore {

    class CSSRuleList;
    class CSSStyleDeclaration;
    class DOMSelection;
    class DOMURL;
    class DOMWindowProperty;
    class Document;
    class Element;
    class EventListener;
    class FloatRect;
    class Frame;
    class MediaQueryList;
    class Navigator;
    class Node;
    class Page;
    class ScheduledAction;
    class StyleMedia;
    class WebKitPoint;
    class DOMWindowCSS;

#if ENABLE(REQUEST_ANIMATION_FRAME)
    class RequestAnimationFrameCallback;
#endif

    struct WindowFeatures;

    typedef int ExceptionCode;

    enum SetLocationLocking { LockHistoryBasedOnGestureState, LockHistoryAndBackForwardList };

    // FIXME: DOMWindow shouldn't subclass FrameDestructionObserver and instead should get to Frame via its Document.
    class DOMWindow : public RefCounted<DOMWindow>
                    , public EventTarget
                    , public ContextDestructionObserver
                    , public FrameDestructionObserver
                    , public Supplementable<DOMWindow> {
    public:
        static PassRefPtr<DOMWindow> create(Document* document) { return adoptRef(new DOMWindow(document)); }
        virtual ~DOMWindow();

        // In some rare cases, we'll re-used a DOMWindow for a new Document. For example,
        // when a script calls window.open("..."), the browser gives JavaScript a window
        // synchronously but kicks off the load in the window asynchronously. Web sites
        // expect that modifications that they make to the window object synchronously
        // won't be blown away when the network load commits. To make that happen, we
        // "securely transition" the existing DOMWindow to the Document that results from
        // the network load. See also SecurityContext::isSecureTransitionTo.
        void didSecureTransitionTo(Document*);

        virtual const AtomicString& interfaceName() const;
        virtual ScriptExecutionContext* scriptExecutionContext() const;

        virtual DOMWindow* toDOMWindow();

        void registerProperty(DOMWindowProperty*);
        void unregisterProperty(DOMWindowProperty*);

        void resetUnlessSuspendedForPageCache();
        void suspendForPageCache();
        void resumeFromPageCache();

        PassRefPtr<MediaQueryList> matchMedia(const String&);

        unsigned pendingUnloadEventListeners() const;

        static bool dispatchAllPendingBeforeUnloadEvents();
        static void dispatchAllPendingUnloadEvents();

        static FloatRect adjustWindowRect(Page*, const FloatRect& pendingChanges);

        // DOM Level 0

        DOMSelection* getSelection();

        Element* frameElement() const;

        void focus(ScriptExecutionContext* = 0);
        void blur();
        void close(ScriptExecutionContext* = 0);
        void stop();

        String btoa(const String& stringToEncode, ExceptionCode&);
        String atob(const String& encodedString, ExceptionCode&);

        bool find(const String&, bool caseSensitive, bool backwards, bool wrap, bool wholeWord, bool searchInFrames, bool showDialog) const;

        bool offscreenBuffering() const;

        int outerHeight() const;
        int outerWidth() const;
        int innerHeight() const;
        int innerWidth() const;
        int screenX() const;
        int screenY() const;
        int screenLeft() const { return screenX(); }
        int screenTop() const { return screenY(); }
        int scrollX() const;
        int scrollY() const;
        int pageXOffset() const { return scrollX(); }
        int pageYOffset() const { return scrollY(); }

        bool closed() const;

        unsigned length() const;

        String name() const;
        void setName(const String&);

        // Self-referential attributes

        DOMWindow* self() const;
        DOMWindow* window() const { return self(); }
        DOMWindow* frames() const { return self(); }

        DOMWindow* opener() const;
        DOMWindow* parent() const;
        DOMWindow* top() const;

        // DOM Level 2 AbstractView Interface

        Document* document() const;

        // CSSOM View Module

        PassRefPtr<StyleMedia> styleMedia() const;

        // DOM Level 2 Style Interface

        PassRefPtr<CSSStyleDeclaration> getComputedStyle(Element*, const String& pseudoElt) const;

        // WebKit extensions

        PassRefPtr<CSSRuleList> getMatchedCSSRules(Element*, const String& pseudoElt, bool authorOnly = true) const;
        double devicePixelRatio() const;

        PassRefPtr<WebKitPoint> webkitConvertPointFromPageToNode(Node*, const WebKitPoint*) const;
        PassRefPtr<WebKitPoint> webkitConvertPointFromNodeToPage(Node*, const WebKitPoint*) const;        

        void scrollBy(int x, int y) const;
        void scrollTo(int x, int y) const;
        void scroll(int x, int y) const { scrollTo(x, y); }

        void moveBy(float x, float y) const;
        void moveTo(float x, float y) const;

        void resizeBy(float x, float y) const;
        void resizeTo(float width, float height) const;

        // WebKit animation extensions
#if ENABLE(REQUEST_ANIMATION_FRAME)
        int requestAnimationFrame(PassRefPtr<RequestAnimationFrameCallback>);
        int webkitRequestAnimationFrame(PassRefPtr<RequestAnimationFrameCallback>);
        void cancelAnimationFrame(int id);
#endif

#if ENABLE(CSS3_CONDITIONAL_RULES)
        DOMWindowCSS* css();
#endif

        // Events
        // EventTarget API
        virtual bool addEventListener(const AtomicString& eventType, PassRefPtr<EventListener>, bool useCapture);
        virtual bool removeEventListener(const AtomicString& eventType, EventListener*, bool useCapture);
        virtual void removeAllEventListeners();

        using EventTarget::dispatchEvent;
        bool dispatchEvent(PassRefPtr<Event> prpEvent, PassRefPtr<EventTarget> prpTarget);

        void dispatchLoadEvent();

        DEFINE_ATTRIBUTE_EVENT_LISTENER(abort);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(beforeunload);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(blur);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(canplay);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(canplaythrough);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(change);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(click);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(contextmenu);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dblclick);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(drag);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragenter);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragleave);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragover);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(dragstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(drop);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(durationchange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(emptied);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(ended);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(error);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(focus);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(hashchange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(input);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(invalid);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keydown);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keypress);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(keyup);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(load);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadeddata);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadedmetadata);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(loadstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(message);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousedown);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseenter);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseleave);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousemove);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseout);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseover);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mouseup);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(mousewheel);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(offline);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(online);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pagehide);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pageshow);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(pause);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(play);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(playing);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(popstate);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(progress);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(ratechange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(reset);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(resize);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(scroll);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(search);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(seeked);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(seeking);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(select);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(stalled);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(submit);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(suspend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(timeupdate);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(unload);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(volumechange);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(waiting);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(webkitbeginfullscreen);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(webkitendfullscreen);

        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationstart, webkitAnimationStart);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationiteration, webkitAnimationIteration);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkitanimationend, webkitAnimationEnd);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(webkittransitionend, webkitTransitionEnd);
        DEFINE_MAPPED_ATTRIBUTE_EVENT_LISTENER(transitionend, transitionend);

        void captureEvents();
        void releaseEvents();

        using RefCounted<DOMWindow>::ref;
        using RefCounted<DOMWindow>::deref;

#if ENABLE(PROXIMITY_EVENTS)
        DEFINE_ATTRIBUTE_EVENT_LISTENER(webkitdeviceproximity);
#endif

#if ENABLE(TOUCH_EVENTS)
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchstart);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchmove);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchend);
        DEFINE_ATTRIBUTE_EVENT_LISTENER(touchcancel);
#endif

        // FIXME: When this DOMWindow is no longer the active DOMWindow (i.e.,
        // when its document is no longer the document that is displayed in its
        // frame), we would like to zero out m_frame to avoid being confused
        // by the document that is currently active in m_frame.
        bool isCurrentlyDisplayedInFrame() const;

        void willDetachDocumentFromFrame();
        void willDestroyCachedFrame();

        void enableSuddenTermination();
        void disableSuddenTermination();

    private:
        explicit DOMWindow(Document*);

        Page* page();
        bool allowedToChangeWindowGeometry() const;

        virtual void frameDestroyed() OVERRIDE;
        virtual void willDetachPage() OVERRIDE;

        virtual void refEventTarget() { ref(); }
        virtual void derefEventTarget() { deref(); }
        virtual EventTargetData* eventTargetData();
        virtual EventTargetData* ensureEventTargetData();

        void resetDOMWindowProperties();
        void disconnectDOMWindowProperties();
        void reconnectDOMWindowProperties();
        void willDestroyDocumentInFrame();

        bool m_suspendedForPageCache;

        HashSet<DOMWindowProperty*> m_properties;

        mutable RefPtr<StyleMedia> m_media;

        EventTargetData m_eventTargetData;

#if ENABLE(CSS3_CONDITIONAL_RULES)
        mutable RefPtr<DOMWindowCSS> m_css;
#endif
    };

} // namespace WebCore

#endif // DOMWindow_h
