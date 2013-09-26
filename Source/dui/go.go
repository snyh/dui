package main

import "dui"

func main() {
    content := `
    <input id="button" type=button value="hello"/>
    1234
    <div style="-webkit-transform: rotate(-20deg);">
    <div id="snyh" style="text-shadow: 2px 2px 2px red; font-size: 18px; -webkit-transition: all 10s ease-in;"> DUI Test </div>
    </div>
    <img style="border: 1px solid blue" height="300" src="/dev/shm/dui/Source/dui/test.gif" />
    `

    dui.Init()

    f := dui.NewFrame(350, 400)
    f.LoadContent(content)

    e := f.Ele("snyh")
    println(e.Content())

    var flag bool
    f.Ele("button").Connect("click", func() {
        flag = !flag
        if flag {
            e.SetContent("one two three")
        } else {
            e.SetContent("three two one")
        }
    })

    dui.Run()
}
