/*
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ScriptController_h
#define ScriptController_h

#include "loader/FrameLoaderTypes.h"
#include "bindings/dui/DOMWrapperWorld.h"
#include "platform/KURL.h"
#include "bindings/ScriptControllerBase.h"
#include "bindings/dui/ScriptValue.h"
#include <wtf/Forward.h>
#include <wtf/RefPtr.h>
#include <wtf/text/TextPosition.h>
#include <wtf/Threading.h>

namespace WebCore {

class HTMLPlugInElement;
class HTMLDocument;
class Frame;
class ScriptSourceCode;
class ScriptValue;
class DOMWindow;
class Widget;
struct JSDOMWindowShell {
};
struct JSDOMWindow {
};

class ScriptController {
    friend class ScriptCachedFrameData;

public:
    ScriptController(Frame*) {};
    //~ScriptController();

    static PassRefPtr<DOMWrapperWorld> createWorld() {return 0;};

    JSDOMWindowShell* createWindowShell(DOMWrapperWorld*) { return 0;};
    void destroyWindowShell(DOMWrapperWorld*) {};

    JSDOMWindowShell* windowShell(DOMWrapperWorld* world)
    {
        return 0;
    }
    JSDOMWindowShell* existingWindowShell(DOMWrapperWorld* world) const
    {
        return 0;
    }
    JSDOMWindow* globalObject(DOMWrapperWorld* world)
    {
        return 0;
    }

    static void getAllWorlds(Vector<RefPtr<DOMWrapperWorld> >&) {};

    ScriptValue executeScript(const ScriptSourceCode&) {return ScriptValue();};
    ScriptValue executeScript(const String& script, bool forceUserGesture = false) {return ScriptValue();};
    ScriptValue executeScriptInWorld(DOMWrapperWorld*, const String& script, bool forceUserGesture = false) {return ScriptValue();};

    // Returns true if argument is a JavaScript URL.
    bool executeIfJavaScriptURL(const KURL&, ShouldReplaceDocumentIfJavaScriptURL shouldReplaceDocumentIfJavaScriptURL = ReplaceDocumentIfJavaScriptURL) {
        return false;
    }

    // This function must be called from the main thread. It is safe to call it repeatedly.
    // Darwin is an exception to this rule: it is OK to call this function from any thread, even reentrantly.
    static void initializeThreading() { WTF::initializeMainThread(); };

    ScriptValue evaluate(const ScriptSourceCode&) {return ScriptValue();};
    ScriptValue evaluateInWorld(const ScriptSourceCode&, DOMWrapperWorld*) {return ScriptValue();};

    WTF::TextPosition eventHandlerPosition() const;

    void enableEval() {};
    void disableEval(const String& errorMessage) {};

    static bool processingUserGesture() {return false;};

    static bool canAccessFromCurrentOrigin(Frame*) {return false;};
    bool canExecuteScripts(ReasonForCallingCanExecuteScripts) {return false;};

    void setPaused(bool b) { }
    bool isPaused() const { return false; }

    const String* sourceURL() const { return 0; } // 0 if we are not evaluating any script

    void clearWindowShell(DOMWindow* newDOMWindow, bool goingIntoPageCache) {};
    void updateDocument() {};

    void namedItemAdded(HTMLDocument*, const AtomicString&) { }
    void namedItemRemoved(HTMLDocument*, const AtomicString&) { }

    // Notifies the ScriptController that the securityOrigin of the current
    // document was modified.  For example, this method is called when
    // document.domain is set.  This method is *not* called when a new document
    // is attached to a frame because updateDocument() is called instead.
    void updateSecurityOrigin() {};

    void clearScriptObjects() {};
    void cleanupScriptObjectsForPlugin(void*) {};

    void updatePlatformScriptObjects() {};
};

} // namespace WebCore

#endif // ScriptController_h
