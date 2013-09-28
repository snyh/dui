package dui

// #cgo CFLAGS: -L/dev/shm/dui/build/.libs
// #cgo LDFLAGS: -ldui
// #include <stdlib.h>
// #include "dui.h"
// #include <stdio.h>
// static void _setup_handler() {
//      void _dui_listener_handler(ListenerInfo*);
//      set_dui_listener_handle(_dui_listener_handler);
// }
import "C"
import "unsafe"
import "reflect"
import "runtime"

func Init() {
    C.d_init()
    C._setup_handler();
}
func Run() {
    C.d_main()
}
func Quit() {
    C.d_main_quit()
}


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
    r := &Element{C.d_frame_get_element(f.Core, cs)}
    C.free(unsafe.Pointer(cs))
    return r
}



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
func (e *Element) Content() string {
    return C.GoString(C.d_element_get_content(e.Core))
}
func (e *Element) SetContent(content string) {
    cs := C.CString(content)
    C.d_element_set_content(e.Core, cs)
    C.free(unsafe.Pointer(cs))
}

type ListenerContext struct {
    f interface{}
    info unsafe.Pointer
    data reflect.Value
}
var element_listeners []*ListenerContext

func (e *Element) Connect(name string, f interface{}, datas ...interface{}) {
    var data interface{}
    if len(datas) > 0 {
        data = datas[0]
    }
    ctx := &ListenerContext{f, nil, reflect.ValueOf(data)}
    cs_name := C.CString(name)
    ctx.info = unsafe.Pointer(C.d_element_add_listener(e.Core, cs_name, C.int(len(element_listeners))))
    C.free(unsafe.Pointer(cs_name))
    element_listeners = append(element_listeners, ctx)
}

//export _dui_listener_handler
func _dui_listener_handler(info *C.ListenerInfo) {
    ctx := element_listeners[int(info.func_id)]
    rf := reflect.ValueOf(ctx.f)
    rf.Call(nil)
}
