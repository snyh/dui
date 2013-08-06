#ifndef ScriptCachedFrameData_h
#define ScriptCachedFrameData_h

namespace WebCore {
    class Frame;
    class ScriptCachedFrameData {
        public:
            ScriptCachedFrameData(Frame*) {}
            void restore(Frame*) {}
            void clear() {}
    };

} // namespace WebCore

#endif // ScriptCachedFrameData_h
