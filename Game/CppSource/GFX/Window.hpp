#ifndef GFX_Window_h__
#define GFX_Window_h__

#include <memory>

#include "SDL_platform.h"

#include "Util/noncopyable.hpp"

struct SDL_Window;

class Window : public util::Noncopyable {
    struct GLContext {
#ifdef __WIN32__
        void* display;
        void* surface;
#endif // __WIN32__
        void* context;

        explicit GLContext(SDL_Window* window);
        ~GLContext();
    };

    std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> _window;
    GLContext _gl;

public:
    Window(const size_t width, const size_t height);

    size_t width() const;
    size_t height() const;

    void swapBuffers();
};
#endif // GFX_Window_h__