#ifndef DOMWrapperWorld_h
#define DOMWrapperWorld_h

#include <wtf/Forward.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class DOMWrapperWorld : public RefCounted<DOMWrapperWorld> {
};

DOMWrapperWorld* mainThreadNormalWorld();

} // namespace WebCore
#endif // DOMWrapperWorld_h
