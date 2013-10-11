#ifndef __DUI_DMAIN_H__
#define __DUI_DMAIN_H__

#ifdef __cplusplus
extern "C" {
#endif
    void d_main();
    void d_main_quit();
    void d_init();


    typedef struct _DFrame {
        void* native;
        void* core;
    } DFrame;
    typedef void DElement;


    DFrame* d_frame_new(int width, int height);
    void d_frame_load_content(DFrame* frame, const char* content);
    DElement* d_frame_get_element(DFrame* frame, const char* id);
    void d_frame_add(DFrame* frame, DElement* ele);

#ifdef __cplusplus
}
#endif


#endif
