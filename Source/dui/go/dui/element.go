package dui
/*
#include "dui.go.h"
*/
import "C"
import "unsafe"
import "runtime"

type Element struct {
    Core unsafe.Pointer
}
func (f *Frame) NewElement(ele_type string) *Element {
    cs := C.CString(ele_type)
    el := Element{C.d_element_new(f.Core, cs)}
    C.free(unsafe.Pointer(cs))
    runtime.SetFinalizer(&el, func(el *Element) { C.d_element_free(el.Core) })
    return &el
}
func (e *Element) Add(child *Element) {
    C.d_element_add(e.Core, child.Core)
}
func (e *Element) Content() string {
    if e.Core != nil {
        return C.GoString(C.d_element_get_content(e.Core))
    }
    return ""
}
func (e *Element) Set(key string, value string) bool {
    cs_key := C.CString(key)
    cs_value := C.CString(value)
    C.d_element_set_attribute(e.Core, cs_key, cs_value)
    C.free(unsafe.Pointer(cs_key))
    C.free(unsafe.Pointer(cs_value))
    return true
}
func (e *Element) Get(key string) string {
    cs_key := C.CString(key)
    defer C.free(unsafe.Pointer(cs_key))

    return C.GoString(C.d_element_get_attribute(e.Core, cs_key))
}
func (e *Element) SetContent(content string) {
    cs := C.CString(content)
    C.d_element_set_content(e.Core, cs)
    C.free(unsafe.Pointer(cs))
}

func (e *Element) SetStyle(id int, value string) {
    cs := C.CString(value)
    defer C.free(unsafe.Pointer(cs))

    C.d_element_set_style(e.Core, C.int(id), cs)
}
