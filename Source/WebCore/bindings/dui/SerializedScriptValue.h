#ifndef SerializedScriptValue_h
#define SerializedScriptValue_h

#include <wtf/RefCounted.h>
#include <wtf/PassRefPtr.h>
#include <wtf/text/WTFString.h>
#include <wtf/Vector.h>

namespace WebCore {

enum SerializationReturnCode {
    SuccessfullyCompleted,
    StackOverflowError,
    InterruptedExecutionError,
    ValidationError,
    ExistingExceptionError,
    DataCloneError,
    UnspecifiedError
};
    
enum SerializationErrorMode { NonThrowing, Throwing };

class SerializedScriptValue : public RefCounted<SerializedScriptValue> {
    public:
        static PassRefPtr<SerializedScriptValue> create() {
            return adoptRef(new SerializedScriptValue());
        }


        static PassRefPtr<SerializedScriptValue> nullValue() {
            return create();
        }
        static PassRefPtr<SerializedScriptValue> undefinedValue() {
            return create();
        }
        static PassRefPtr<SerializedScriptValue> booleanValue(bool value) {
            return create();
        }
        static PassRefPtr<SerializedScriptValue> adopt(Vector<uint8_t>& buffer)
        {
            return adoptRef(new SerializedScriptValue());
        }
        static PassRefPtr<SerializedScriptValue> create(const String&) {
            return create();
        }
        const Vector<uint8_t>& data() { return m_data; }
        const Vector<String>& blobURLs() const { return m_blobURLs; }
        const Vector<uint8_t>& toWireBytes() const { return m_data; }
    private:
        Vector<unsigned char> m_data;
        Vector<String> m_blobURLs;
};

}

#endif // SerializedScriptValue_h
