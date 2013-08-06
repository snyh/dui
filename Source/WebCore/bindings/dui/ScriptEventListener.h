#ifndef ScriptEventListener_h
#define ScriptEventListener_h

#include "bindings/dui/ScriptState.h"
#include "bindings/dui/ScriptValue.h"

#include <wtf/PassRefPtr.h>
#include "dom/EventListener.h"

namespace WebCore {
    class ScriptExecutionContext;
    class Attribute;
    class EventListener;
    class Frame;
    class Node;

    class DummyEventListener : public EventListener {
        public:
        virtual bool operator==(const EventListener&) { return false; }
        virtual void handleEvent(ScriptExecutionContext*, Event*) {}
        DummyEventListener(): EventListener(EventListener::CPPEventListenerType)  {};
    };

    static const DummyEventListener* cast(const EventListener* l) {
        return 0;
    }

    inline PassRefPtr<DummyEventListener> createAttributeEventListener(Node*, const QualifiedName&, const AtomicString& value) {
        return adoptRef(new DummyEventListener());
    }
    inline PassRefPtr<DummyEventListener> createAttributeEventListener(Frame*, const QualifiedName&, const AtomicString& value) {
        return adoptRef(new DummyEventListener());
    }
    String eventListenerHandlerBody(Document*, EventListener*);
    ScriptValue eventListenerHandler(Document*, EventListener*);
    ScriptState* eventListenerHandlerScriptState(Frame*, EventListener*);
    bool eventListenerHandlerLocation(Document*, EventListener*, String& sourceName, String& scriptId, int& lineNumber);
} // namespace WebCore

#endif // ScriptEventListener_h
