#ifndef DEventListener_h
#define DEventListener_h

#include <wtf/PassRefPtr.h>
#include "dom/EventListener.h"
#include <wtf/text/AtomicString.h>

extern "C" {
        // must sync with dui/api/event.h
    typedef struct _ListenerInfo ListenerInfo;
    typedef void (*DListenerHandler)(ListenerInfo*, void* event);
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
        static PassRefPtr<DEventListener> create(ListenerInfo* info)
        {
            return adoptRef(new DEventListener(info));
        }

        private:
            DEventListener(ListenerInfo*);
            ListenerInfo* m_info;
    };

    inline PassRefPtr<DEventListener> createAttributeEventListener(Node*, const QualifiedName&, const AtomicString& value) {
        return DEventListener::create(0);
    }
    inline PassRefPtr<DEventListener> createAttributeEventListener(Frame*, const QualifiedName&, const AtomicString& value) {
        return DEventListener::create(0);
    }
} // namespace WebCore

#endif // DEventListener_h
