package main

import "dui"
import "fmt"
import "io/ioutil"

func main() {
    dui.Init()

    f := dui.NewFrame(300, 300)
    content, _ := ioutil.ReadFile("/dev/shm/dui/Source/dui/test.htm")
    f.LoadContent(string(content))

    e := f.Ele("snyh")
    fmt.Printf(e.Content())

    var flag bool
    f.Ele("button").SignalConnect("click", func() {
        if flag {
            e.SetContent("one two three")
            flag = false
        } else {
            e.SetContent("three two one")
            flag = true
        }
    })

    dui.Run()
}
