package main

// #cgo CFLAGS: -L/dev/shm/dui/build/.libs
// #cgo LDFLAGS: -ldui
// #include "dui.h"
import "C"
import "io/ioutil"

func main() {
    C.d_init()
    f := C.d_frame_new(300, 300)
    content, _ := ioutil.ReadFile("test.htm")
    C.d_frame_load_content(f, C.CString(string(content)))
    C.d_main()
}
