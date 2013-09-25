package dui

// #cgo CFLAGS: -L/dev/shm/dui/build/.libs
// #cgo LDFLAGS: -ldui
// #include "dui.h"
// #include <stdio.h>
// static void _setup_handler() {
//      void _dui_listener_handler(ListenerInfo*);
//     printf("handler:%p\n", _dui_listener_handler);
//      set_dui_listener_handle(_dui_listener_handler);
// }
import "C"
import "unsafe"
import "reflect"

func Init() {
    C.d_init()
    C._setup_handler();
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
func (f *Frame) Ele(id string) *Element {
    return  &Element{C.d_frame_get_element(f.Core, C.CString(id))}
}



type Element struct {
    Core unsafe.Pointer
}
func (e *Element) Content() string {
    return C.GoString(C.d_element_get_content(e.Core))
}
func (e *Element) SetContent(content string) {
    C.d_element_set_content(e.Core, C.CString(content))
}

type ListenerContext struct {
    f interface{}
    info unsafe.Pointer
    data reflect.Value
}
var element_listeners []*ListenerContext

func (e *Element) SignalConnect(name string, f interface{}, datas ...interface{}) {
    var data interface{}
    if len(datas) > 0 {
        data = datas[0]
    }
    ctx := &ListenerContext{f, nil, reflect.ValueOf(data)}
    ctx.info = unsafe.Pointer(C.d_element_add_listener(e.Core, C.CString(name), C.int(len(element_listeners))))
    element_listeners = append(element_listeners, ctx)
}

//export _dui_listener_handler
func _dui_listener_handler(info *C.ListenerInfo) {
    ctx := element_listeners[int(info.func_id)]
    rf := reflect.ValueOf(ctx.f)
    rf.Call(nil)
}
