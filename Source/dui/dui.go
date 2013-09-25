package dui


// #cgo CFLAGS: -L/dev/shm/dui/build/.libs
// #cgo LDFLAGS: -ldui
// #include "dui.h"
import "C"

func Init() {
    C.d_init()
}
func Run() {
    C.d_main()
}



type Frame struct {
    Core *C.DFrame;
}
func NewFrame(width, height int) *Frame {
    return &Frame{C.d_frame_new(C.int(width), C.int(height))}
}
func (f *Frame) LoadContent(content string) {
    C.d_frame_load_content(f.Core, C.CString(string(content)))
}
func (f *Frame) GetElement(id string) *Element {
    return  &Element{C.d_frame_get_element(f.Core, C.CString(id))}
}



type Element struct {
    Core *C.DElement
}
func (e *Element) GetContent() string {
    return C.GoString(C.d_element_get_content(e.Core))
}
func (e *Element) SetContent(content string) {
    C.d_element_set_content(e.Core, C.CString(content))
}
func (e *Element) SignalConnect(name string, f interface{}) int {
    return 0
}

