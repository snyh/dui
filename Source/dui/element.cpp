#include "config.h"
#include "dom/Element.h"
#include "page/Frame.h"
#include <wtf/text/CString.h>

#include "bindings/dui/DEventListener.h"
#include "api/element.h"
#include "api/event.h"
#include <glib.h>

using namespace WebCore;

void d_element_add(DElement* self, DElement* child)
{
    ((Element*)self)->appendChild((Element*)child);
}

DElement* d_element_new(DFrame* dframe, const char* type)
{
    Frame* frame = (Frame*)dframe->core;
    RefPtr<Element> el = frame->document()->createElement(type, IGNORE_EXCEPTION);
    return (DElement*)el.release().leakRef();
}

void _element_free(void* element)
{
    ((Element*)element)->deref();
}
void d_element_free(DElement* element)
{
    g_assert(element != 0);
    callOnMainThread(_element_free, element);
}


const char* d_element_get_content(DElement* element)
{
    Element* e = (Element*)element;
    return e->textContent().utf8().data();
}
const char* d_element_set_content(DElement* element, const char* content)
{
    Element* e = (Element*)element;
    e->setTextContent(content, IGNORE_EXCEPTION);
}
void d_element_set_attribute(DElement* element, const char* key, const char* value)
{
    Element* e = (Element*)element;
    e->setAttribute(key, value, IGNORE_EXCEPTION);
}
const char* d_element_get_attribute(DElement* element, const char* key)
{
    Element* e = (Element*)element;
    return e->getAttribute(key).string().utf8().data();
}

ListenerInfo* d_element_add_listener(DElement* element, const char* type, int id)
{
    Element* e = (Element*)element;
    ListenerInfo* info = g_new(ListenerInfo, 1);
    PassRefPtr<DEventListener> listener = DEventListener::create(info);
    e->addEventListener(type, listener, false);

    info->func_id = id;
    return info;
}
