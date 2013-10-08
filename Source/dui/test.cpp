#include "api/dui.h"
#include <glib.h>


const char* content = "<input id=button type=button value=\"hello\"/>"
"1234"
"<div style=\"-webkit-transform: rotate(-20deg);\">"
"<div id=snyh style=\"text-shadow: 2px 2px 2px red; font-size: 18px; -webkit-transition: all 10s ease-in;\"> DUI Test </div>"
"</div>"
"<img style=\"border: 1px solid blue\" height=300 src=\"/dev/shm/dui/Source/dui/test.gif\" />";

int main()
{
    d_init();

    DFrame* frame = d_frame_new(300, 300);

    d_frame_load_content(frame, content);

    d_main();
}
