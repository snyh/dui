#include "dui.h"
#include <glib.h>

int main()
{
    d_init();

    DFrame* frame = d_frame_new(300, 300);

    char* content = NULL;
    g_file_get_contents("/dev/shm/dui/Source/dui/test.htm", &content, NULL, NULL);
    d_frame_load_content(frame, content);
    g_free(content);

    d_main();
}
