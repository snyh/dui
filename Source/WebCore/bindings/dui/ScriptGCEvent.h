#ifndef ScriptGCEvent_h
#define ScriptGCEvent_h

namespace WebCore {

struct HeapInfo {
    HeapInfo()
        : usedJSHeapSize(0)
        , totalJSHeapSize(0)
        , jsHeapSizeLimit(0)
    {
    }

    size_t usedJSHeapSize;
    size_t totalJSHeapSize;
    size_t jsHeapSizeLimit;
};

} // namespace WebCore

#endif // !defined(ScriptGCEvent_h)
