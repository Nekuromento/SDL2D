#ifndef Core_Application_h__
#define Core_Application_h__

#include <cstdlib>

#include "Util/defines.hpp"
#include "Util/noncopyable.hpp"

class DoubleEndedLinearAllocator;
class Window;

class Application : public util::Noncopyable {
    typedef DoubleEndedLinearAllocator AppAlloc;

    size_t _updateTps;
    size_t _renderFps;
    int64_t _lastTime;
    int64_t _newTime;
    int64_t _lastSecond;
    bool _done;

    void mainLoop(Window* window, AppAlloc& alloc);

public:
    Application() NOEXCEPT;
    ~Application();

    void run();
};
#endif // Core_Application_h__
