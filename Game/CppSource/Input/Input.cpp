#include "Input.hpp"

#include "SDL_events.h"

#include "Core/Memory/LinearAllocator.hpp"
#include "Core/Memory/ScopeStack.hpp"
#include "GFX/Window.hpp"

static const size_t ScratchBufferSize = 32;

void Input::handleExitRequest(ScopeAlloc& scratch) const NOEXCEPT {
    applicationExitRequested.invoke(scratch);
}

void Input::handleKeyPress(ScopeAlloc& scratch,
                           const SDL_KeyboardEvent& event) const NOEXCEPT {
    //HACK: interpret some keystrokes as request for exit
    if (event.keysym.sym == SDLK_ESCAPE || event.keysym.sym == SDLK_AC_BACK) {
        handleExitRequest(scratch);
        return;
    }

    const KeyState keyState = {
        static_cast<wchar_t>(event.keysym.sym),
        event.state == SDL_PRESSED
    };
    keyPressChanged.invoke(scratch, keyState);
}

void Input::handleMouseButton(ScopeAlloc& scratch,
                              const SDL_MouseButtonEvent& event) const NOEXCEPT {
    const CursorState cursorState = {
        event.x,
        event.y,
        event.state == SDL_PRESSED
    };
    if (event.button == 1)
        cursorPressChanged.invoke(scratch, cursorState);
}

void Input::handleMouseMotion(ScopeAlloc& scratch,
                              const SDL_MouseMotionEvent& event) const NOEXCEPT {
    const CursorState cursorState = {
        event.x,
        event.y,
        (event.state & SDL_BUTTON(1)) == 1
    };
    cursorMoved.invoke(scratch, cursorState);
}

void Input::handleFinger(ScopeAlloc& scratch,
                         const Window* window,
                         const int32_t type,
                         const SDL_TouchFingerEvent& event) const NOEXCEPT {

    const CursorState cursorState = {
        (int32_t) (window->width() * event.x),
        (int32_t) (window->height() * event.y),
        type != SDL_FINGERUP
    };
    if (event.fingerId == 0)
        cursorPressChanged.invoke(scratch, cursorState);
}

void Input::processEvents(const Window* window) NOEXCEPT {
    assert(window);

    uint8_t scratchBuffer[ScratchBufferSize];
    LinearAllocator scratch(std::begin(scratchBuffer), std::end(scratchBuffer));

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ScopeAlloc scope(scratch);

        switch (event.type) {
        case SDL_WINDOWEVENT_CLOSE:
        case SDL_QUIT:
            handleExitRequest(scope);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            handleKeyPress(scope, event.key);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            handleMouseButton(scope, event.button);
            break;
        case SDL_MOUSEMOTION:
            handleMouseMotion(scope, event.motion);
            break;
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
            handleFinger(scope, window, event.type, event.tfinger);
        }
    }
}
