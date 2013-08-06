#ifndef ScheduledAction_h
#define ScheduledAction_h

#include <wtf/PassOwnPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

namespace JSC {
    class JSGlobalObject;
}

namespace WebCore {

    class Document;
    class ContentSecurityPolicy;
    class ScriptExecutionContext;
    class WorkerContext;

   /* An action (either function or string) to be executed after a specified
    * time interval, either once or repeatedly. Used for window.setTimeout()
    * and window.setInterval()
    */
    class ScheduledAction {
        WTF_MAKE_NONCOPYABLE(ScheduledAction); WTF_MAKE_FAST_ALLOCATED;
    public:
        static PassOwnPtr<ScheduledAction> create() {}

        void execute(ScriptExecutionContext*) {}
    };

} // namespace WebCore

#endif // ScheduledAction_h
