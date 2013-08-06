package main

import (
    "io/ioutil"
    "strings"
    "path/filepath"
    "fmt"
    "regexp"
)

func get_map() map[string]string {
    b, err := ioutil.ReadFile("file_list")
    if err != nil { panic(err) }

    m := make(map[string]string)

    lines := strings.Split(string(b), "\n")
    for _, s := range lines {
        name := filepath.Base(s)
        if strings.HasSuffix(name, "h") {
            m[name] = s
        }
    }
    return m
}

func check_and_replace_line(line string, m map[string]string) (string, bool) {
    re := regexp.MustCompile("\\s?#include\\s+\"(.*\\.h)\"\\s?$")
    matches := re.FindStringSubmatch(line)
    if len(matches) == 2 && m[matches[1]] != "" {
        return fmt.Sprintf("#include \"%s\"", m[matches[1]]), true
    }
    return line, false
}

func handle_file(filename string, m map[string]string) {
    count := 0
    content, err := ioutil.ReadFile(filename)
    if err != nil {
        panic(err)
    }
    lines := strings.Split(string(content), "\n")
    for i, l := range lines {
        handled := false
        lines[i], handled = check_and_replace_line(l, m)
        if handled {
            count++
        }

    }
    ioutil.WriteFile(filename, []byte(strings.Join(lines, "\n")), 0x777)
    fmt.Printf("Handle file %s (%d)\n", filename, count)
}

func main() {
    b, err := ioutil.ReadFile("file_list")
    if err != nil { panic(err) }

    m := get_map()

    lines := strings.Split(string(b), "\n")
    for _, s := range lines {
        name := filepath.Base(s)
        if strings.HasSuffix(name, "h") || strings.HasSuffix(name, "cpp") {
            handle_file(s, m)
        }
    }
}
