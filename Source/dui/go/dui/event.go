package dui
/*
#cgo LDFLAGS: -ldui

#include "dui.go.h"
static void _setup_handler() {
    void _dui_listener_handler(ListenerInfo*, void*);
    set_dui_listener_handle(_dui_listener_handler);
}

*/
import "C"
import "unsafe"
import "reflect"

type MouseEvent struct {
    X, Y int
}
type KeyboardEvent struct {
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

func NewEvent(p unsafe.Pointer) reflect.Value {
    e := C.d_event_new(p)
    /*fmt.Print("NewEVent: ", e)*/
    return reflect.ValueOf(MouseEvent{int(e.x), int(e.y)})
}


//export _dui_listener_handler
func _dui_listener_handler(info *C.ListenerInfo, e unsafe.Pointer) {
    ctx := element_listeners[int(info.func_id)]
    rf := reflect.ValueOf(ctx.f)
    rf.Call([]reflect.Value{NewEvent(e)})
}
func event_handler_init() {
    C._setup_handler()
}
