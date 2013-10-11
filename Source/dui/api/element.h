#ifndef __DUI_ELEMENT_H__
#define __DUI_ELEMENT_H__

#include "dmain.h"

#ifdef __cplusplus
extern "C" {
#endif


void d_element_free(DElement* element);
DElement* d_element_new(DFrame* frame, const char* type);
void d_element_add(DElement* self, DElement* child);
void d_element_set_attribute(DElement*, const char* key, const char* value);
const char* d_element_get_attribute(DElement*, const char* key);
const char* d_element_get_content(DElement*);
const char* d_element_set_content(DElement*, const char*);

#ifdef __cplusplus
}
#endif

#endif
