package dui
/*
#include "dui.h"
#include <stdlib.h>
*/
import "C"
import "unsafe"

type Frame struct {
    Core *C.DFrame;
}
func NewFrame(width, height int) *Frame {
    return &Frame{C.d_frame_new(C.int(width), C.int(height))}
}
func (f *Frame) LoadContent(content string) {
    cs := C.CString(content)
    C.d_frame_load_content(f.Core, cs)
    C.free(unsafe.Pointer(cs))
}
func (f *Frame) Ele(id string) *Element {
    cs := C.CString(id)
    defer C.free(unsafe.Pointer(cs))

    ele_core := C.d_frame_get_element(f.Core, cs)
    if ele_core != nil {
        return &Element{ele_core}
    } else {
        return nil
    }
}

func (f *Frame) Add(child *Element) {
    C.d_frame_add(f.Core, child.Core)
}


