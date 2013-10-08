package dui

/*
#include "dui.go.h"
*/
import "C"

func Init() {
    C.d_init()
    event_handler_init()
}
func Run() {
    C.d_main()
}
func Quit() {
    C.d_main_quit()
}
