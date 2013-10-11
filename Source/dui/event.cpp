#include "config.h"

#include "api/dui.h"

#include "dom/Event.h"
#include "dom/MouseEvent.h"

DEvent d_event_new(void* _e)
{
    WebCore::Event* event = (WebCore::Event*)_e;
    DEvent r;
    if (event->isMouseEvent()) {
        WebCore::MouseEvent* me = static_cast<WebCore::MouseEvent*>(event);
        r.x = me->x();
        r.y = me->y();
        r.which = me->which();
    } else {
        printf("Not an MouseEvent\n");
    }
    return r;
}
