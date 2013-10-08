
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DEvent {
    int x;
    int y;
    int which;
    int type;
} DEvent;

typedef struct _ListenerInfo {
    int func_id;
    void* listener;
    DEvent* event;
} ListenerInfo;

//must sync with DEventListener.h
typedef void (*DListenerHandler)(ListenerInfo*, void* event);
void set_dui_listener_handle(DListenerHandler);

ListenerInfo* d_element_add_listener(DElement* element, const char* type, int id);

DEvent d_event_new(void*);


#ifdef __cplusplus
}

#endif
