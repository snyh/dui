#include "config.h"
#include "DEventListener.h"

static DListenerHandler _handler = 0;
namespace WebCore {
    DEventListener::DEventListener(ListenerInfo* info)
        : EventListener(EventListener::CPPEventListenerType),
          m_info(info)
    {
    }

    bool DEventListener::operator==(const EventListener&)
    {
        return false;
    }
    void DEventListener::handleEvent(ScriptExecutionContext*, Event* e)
    {
        ASSERT(_handler != 0);
        _handler(m_info);
    }
}

void set_dui_listener_handle(DListenerHandler handler)
{
    ASSERT(_handler == 0);
    _handler = handler;
    ASSERT(_handler != 0);
}
