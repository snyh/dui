package main

import "dui"
import "fmt"
import "io/ioutil"

func main() {
    dui.Init()

    f := dui.NewFrame(300, 300)
    content, _ := ioutil.ReadFile("/dev/shm/dui/Source/dui/test.htm")
    f.LoadContent(string(content))

    e := f.GetElement("snyh")
    fmt.Printf(e.GetContent())
    e.SetContent("one two three")

    dui.Run()
}
