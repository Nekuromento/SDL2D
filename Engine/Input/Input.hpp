#ifndef Input_Input_h__
#define Input_Input_h__

#include <cstdint>

#include "Core/Event.hpp"
#include "Util/noncopyable.hpp"

class LinearAllocator;
template <typename Allocator>
class ScopeStack;

class Window;
struct SDL_KeyboardEvent;
struct SDL_MouseButtonEvent;
struct SDL_MouseMotionEvent;
struct SDL_TouchFingerEvent;

class Input : public util::Noncopyable {
    typedef ScopeStack<LinearAllocator> ScopeAlloc;

    void handleExitRequest(ScopeAlloc& scratch) const NOEXCEPT;
    void handleKeyPress(ScopeAlloc& scratch,
                        const SDL_KeyboardEvent& key) const NOEXCEPT;
    void handleMouseButton(ScopeAlloc& scratch,
                           const SDL_MouseButtonEvent& button) const NOEXCEPT;
    void handleMouseMotion(ScopeAlloc& scratch,
                           const SDL_MouseMotionEvent& motion) const NOEXCEPT;
    void handleFinger(ScopeAlloc& scratch,
                      const Window* window,
                      const int32_t type,
                      const SDL_TouchFingerEvent& event) const NOEXCEPT;

public:
    struct CursorState {
        const int32_t x;
        const int32_t y;
        const bool pressed;
    };

    struct KeyState {
        const wchar_t key;
        const bool pressed;
    };

    template <typename Allocator>
    explicit Input(Allocator& alloc) NOEXCEPT :
        cursorPressChanged {alloc},
        cursorMoved {alloc},
        keyPressChanged {alloc},
        applicationExitRequested {alloc}
    {}

    void processEvents(const Window* window) NOEXCEPT;
    typedef Event<void ()> ExitEvent;
    typedef Event<void (const CursorState&)> CursorEvent;
    typedef Event<void (const KeyState&)> KeyEvent;

    CursorEvent cursorPressChanged;
    CursorEvent cursorMoved;
    KeyEvent keyPressChanged;
    ExitEvent applicationExitRequested;
};

#endif // Input_Input_h__