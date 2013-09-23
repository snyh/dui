package main

// #cgo CFLAGS: -L/dev/shm/dui/build/.libs
// #cgo LDFLAGS: -ldui
// #include "dui.h"
import "C"
import "fmt"
import "io/ioutil"

func main() {
    C.d_init()
    f := C.d_frame_new(300, 300)
    content, _ := ioutil.ReadFile("/dev/shm/dui/Source/dui/test.htm")
    C.d_frame_load_content(f, C.CString(string(content)))

    e := C.d_frame_get_element(f, C.CString("snyh"))
    s := C.d_element_get_content(e)
    fmt.Print(C.GoString(s))

    C.d_element_set_content(e, C.CString("one two three"))

    C.d_main()
}
