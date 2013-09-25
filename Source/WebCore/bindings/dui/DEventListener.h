#ifndef DEventListener_h
#define DEventListener_h

#include <wtf/PassRefPtr.h>
#include "dom/EventListener.h"
#include <wtf/text/AtomicString.h>

extern "C" {
    typedef struct _ListenerInfo ListenerInfo;
    typedef void (*DListenerHandler)(ListenerInfo*);
    void set_dui_listener_handle(DListenerHandler);
}

namespace WebCore {
    class ScriptExecutionContext;
    class Attribute;
    class EventListener;
    class QualifiedName;
    class Frame;
    class Node;

    class DEventListener : public EventListener {
        public:
            virtual bool operator==(const EventListener&);
            virtual void handleEvent(ScriptExecutionContext*, Event*);
            DEventListener();
            DEventListener(ListenerInfo*);
        private:
            ListenerInfo* m_info;
    };

    inline PassRefPtr<DEventListener> createAttributeEventListener(Node*, const QualifiedName&, const AtomicString& value) {
        return adoptRef(new DEventListener());
    }
    inline PassRefPtr<DEventListener> createAttributeEventListener(Frame*, const QualifiedName&, const AtomicString& value) {
        return adoptRef(new DEventListener());
    }
} // namespace WebCore

#endif // DEventListener_h
