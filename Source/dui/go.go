package main

import "dui"
import "runtime"

func main() {
    content := `
    <input id="button" type=button value="hello"/>
    1234
    <div style="-webkit-transform: rotate(-20deg);">
    <div id="snyh" style="text-shadow: 2px 2px 2px red; font-size: 18px; -webkit-transition: all 10s ease-in;"> DUI Test </div>
    </div>
    <img style="border: 1px solid blue" height="300" src="/dev/shm/dui/Source/dui/test.gif" />
    <input id="quit" type=button value="exit"/>
    `

    dui.Init()

    f := dui.NewFrame(350, 400)
    f.LoadContent(content)

    e := f.Ele("snyh")
    println(e.Content())

    m := f.NewElement("div")
    m.SetContent("AAAAAAAAAAAAAAAAAAA")

    var flag bool
    f.Ele("button").Connect("click", func() {
        flag = !flag
        if flag {
            e.SetContent("one two three")
            println(e.Content())
        } else {
            e.SetContent("three two one")
            println(e.Content())
        }
        runtime.GC()
    })

    /*f.Ele("quit").Connect("click", dui.Quit)*/
    f.Ele("quit").Connect("click", func() {
        for i:=0; i<100000; i++{
            ee := f.NewElement("div")
            /*ee.SetContent("AAAAAAAAAAAAAAAAAAA")*/
            /*print(ee.Content())*/
            ee = ee
            runtime.GC()
        }
        runtime.GC()
    })
    runtime.GC()

    dui.Run()
}
