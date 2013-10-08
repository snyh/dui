package main

import "./dui"
import "runtime"
import "reflect"
import "fmt"

func main() {
    dui.Init()

    f := dui.NewFrame(350, 500)

    img := f.NewElement("img")
    img.Set("style", "border: 1px solid blue")
    img.Set("height", "300")
    img.Set("src", "/dev/shm/dui/Source/dui/test.gif")
    f.Add(img)
    print("imgWidth:", img.Get("height"), "\n")

    var a dui.Frame
    t := reflect.ValueOf(a)
    fmt.Print(t)

    txt := f.NewElement("div")
    txt.Set("style", "text-shadow: 2px 2px 2px red; font-size: 18px; -webkit-transform: rotate(-20deg);")
    txt.SetContent("DUI Test")
    f.Add(txt)

    quit := f.NewElement("input")
    quit.Set("type", "button")
    quit.Set("value", "exit")
    quit.Set("id", "quit")
    f.Add(quit)
    f.Ele("quit").Connect("click", dui.Quit)

    hello := f.NewElement("input")
    hello.Set("type", "button")
    hello.Set("value", "hello")
    hello.Set("id", "hello")
    f.Add(hello)

    var flag bool
    hello.Connect("click", func(e bool) {
        flag = !flag
        if flag {
            txt.SetContent("one two three")
            println(txt.Content())
        } else {
            txt.SetContent("three two one")
            println(txt.Content())
        }
        runtime.GC()
    })

    dui.Run()
}
