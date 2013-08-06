#ifndef ScriptValue_h
#define ScriptValue_h

#include "bindings/dui/SerializedScriptValue.h"
#include "bindings/dui/ScriptState.h"
#include <wtf/PassRefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class ScriptValue {
public:
    ScriptValue() { }
    virtual ~ScriptValue() {}

    bool getString(ScriptState*, String& result) const { return false;};
    String toString(ScriptState*) const { return "";};
    bool isEqual(ScriptState*, const ScriptValue&) const { return false;};
    bool isNull() const { return true;};
    bool isUndefined() const { return true;};
    bool isObject() const { return false;};
    bool isFunction() const { return false;};
    bool hasNoValue() const { return true; }
    void clear() { }

    bool operator==(const ScriptValue& other) const { return false; }
};

} // namespace WebCore

#endif // ScriptValue_h
