#include "bindings/dui/ScriptState.h"

namespace WebCore {
    DOMWindow* domWindowFromScriptState(ScriptState*) 
    {
        return 0;
    };
    ScriptExecutionContext* scriptExecutionContextFromScriptState(ScriptState*) 
    {
        return 0;
    };

    bool evalEnabled(ScriptState*) {
        return false;
    };
    void setEvalEnabled(ScriptState*, bool) 
    {
    };

    ScriptState* mainWorldScriptState(Frame*)
    {
        return 0;
    }

    ScriptState* scriptStateFromNode(DOMWrapperWorld*, Node*) 
    {
        return 0;
    };

    ScriptState* scriptStateFromPage(DOMWrapperWorld*, Page*) 
    {
        return 0;
    };
}
