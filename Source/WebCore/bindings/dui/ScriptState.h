#ifndef ScriptState_h
#define ScriptState_h

#include <wtf/Noncopyable.h>

namespace WebCore {
class DOMWindow;
class DOMWrapperWorld;
class Frame;
class Node;
class Page;
class ScriptExecutionContext;
class WorkerContext;

// The idea is to expose "state-like" methods (hadException, and any other
// methods where ExecState just dips into globalData) of JSC::ExecState as a
// separate abstraction.
// For now, the separation is purely by convention.
struct ScriptState {
    bool hadException() const { return false;}
};

class ScriptStateProtectedPtr {
    WTF_MAKE_NONCOPYABLE(ScriptStateProtectedPtr);
public:
    explicit ScriptStateProtectedPtr(ScriptState*) {};
    ~ScriptStateProtectedPtr() {};
    ScriptState* get() const {return 0;};
};

DOMWindow* domWindowFromScriptState(ScriptState*);
ScriptExecutionContext* scriptExecutionContextFromScriptState(ScriptState*);

bool evalEnabled(ScriptState*);
void setEvalEnabled(ScriptState*, bool);

ScriptState* mainWorldScriptState(Frame*);

ScriptState* scriptStateFromNode(DOMWrapperWorld*, Node*);
ScriptState* scriptStateFromPage(DOMWrapperWorld*, Page*);

} // namespace WebCore

#endif // ScriptState_h
