#ifndef __DUI_H__
#define __DUI_H__
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




    void d_element_free(DElement* element);
    DElement* d_element_new(DFrame* frame, const char* type);
    void d_element_add(DElement* self, DElement* child);
    void d_element_set_attribute(DElement*, const char* key, const char* value);
    const char* d_element_get_attribute(DElement*, const char* key);
    const char* d_element_get_content(DElement*);
    const char* d_element_set_content(DElement*, const char*);



    typedef struct _ListenerInfo {
        int func_id;
        void* listener;
    } ListenerInfo;
    typedef void (*DListenerHandler)(ListenerInfo*);
    void set_dui_listener_handle(DListenerHandler);

    ListenerInfo* d_element_add_listener(DElement* element, const char* type, int id);

#ifdef __cplusplus
}
#endif


#endif
