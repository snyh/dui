package dui
import "runtime"

/*
#include "dui.go.h"
*/
import "C"

func init() {
    C.d_init()
    event_handler_init()

    runtime.LockOSThread()
    go func() {
        for f := range mainfunc {
            f()
        }
    }()
}

var mainfunc = make(chan func())
func Run() {
    C.d_main()
}
func Quit() {
    C.d_main_quit()
}


func Do(f func()) {
    done := make(chan bool, 1)
    mainfunc <- func() {
        f()
        done <- true
    }
    <-done
}
