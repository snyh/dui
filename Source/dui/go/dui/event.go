package dui
/*
#cgo CFLAGS: -L/dev/shm/dui/build/.libs
#cgo LDFLAGS: -ldui

#include <stdlib.h>
#include "dui.h"
#include <stdio.h>
static void _setup_handler() {
    void _dui_listener_handler(ListenerInfo*);
    set_dui_listener_handle(_dui_listener_handler);
}

*/
import "C"
import "unsafe"
import "reflect"

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
    /*e := NewEvent(info.event)*/
    rf.Call(nil)
}
func event_handler_init() {
    C._setup_handler()
}
