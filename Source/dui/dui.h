#ifndef __DUI_H__
#define __DUI_H__
#ifdef __cplusplus
extern "C" {
#endif
    typedef struct _DFrame {
        void* native;
        void* core;
    } DFrame;

    DFrame* d_frame_new(int width, int height);
    void d_frame_load_content(DFrame* frame, const char* content);
    void d_main();
    void d_init();
#ifdef __cplusplus
}
#endif


#endif
