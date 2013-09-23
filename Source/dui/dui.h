#ifndef __DUI_H__
#define __DUI_H__
#ifdef __cplusplus
extern "C" {
#endif
    typedef struct _DFrame {
        void* native;
        void* core;
    } DFrame;

    typedef void DElement;

    DFrame* d_frame_new(int width, int height);
    void d_frame_load_content(DFrame* frame, const char* content);
    void d_main();
    void d_init();

    DElement* d_frame_get_element(DFrame* frame, const char* id);
    const char* d_element_get_content(DElement*);
    const char* d_element_set_content(DElement*, const char*);


#ifdef __cplusplus
}
#endif


#endif
