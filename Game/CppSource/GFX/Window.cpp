#include "Window.hpp"

#include <cassert>

#include "SDL_video.h"

#ifdef __WIN32__
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  include "SDL_syswm.h"

#  include "EGL/egl.h"
#  include "EGL/eglext.h"

HWND getWindowHandle(SDL_Window* window) {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    SDL_bool gotInfo = SDL_GetWindowWMInfo(window, &wmInfo);
    assert(gotInfo);

    return wmInfo.info.win.window;
}
#endif // __WIN32__

Window::GLContext::GLContext(SDL_Window* window) :
#ifdef __WIN32__
    display {nullptr},
    surface {nullptr},
#endif
    context {nullptr}
{
#ifdef __WIN32__
    HWND windowHandle = getWindowHandle(window);

    display = eglGetDisplay(GetDC(windowHandle));
    assert(display != EGL_NO_DISPLAY);

    EGLint majorVersion, minorVersion;
    const auto initialized = eglInitialize(display, &majorVersion, &minorVersion);
    assert(initialized);

    EGLint configCount;
    const auto gotConfigs = eglGetConfigs(display, nullptr, 0, &configCount);
    assert(gotConfigs);

    EGLint configAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, EGL_DONT_CARE,
        EGL_STENCIL_SIZE, EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE, EGL_NONE
    };

    EGLConfig config;
    const auto choseConfig =
        eglChooseConfig(display, configAttributes, &config, 1, &configCount);
    assert(choseConfig);

    EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE, EGL_NONE
    };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    assert(context != EGL_NO_CONTEXT);

    EGLint surfaceAttributes[] = {
        EGL_NONE, EGL_NONE
    };
    surface =
        eglCreateWindowSurface(display, config,  windowHandle, surfaceAttributes);
    assert(surface != EGL_NO_SURFACE);

    const auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);
#else
    context = SDL_GL_CreateContext(window);
    assert(context);
#endif // __WIN32__
}

Window::GLContext::~GLContext() {
#ifdef __WIN32__
    if (surface != EGL_NO_SURFACE)
        eglDestroySurface(display, surface);
    if (context != EGL_NO_CONTEXT)
        eglDestroyContext(display, context);
    if (display != EGL_NO_DISPLAY)
        eglTerminate(display);
#else
    SDL_GL_DeleteContext(context);
#endif // __WIN32__
}

SDL_Window* createWindow(const char* title, size_t width, size_t height) {
    auto window = SDL_CreateWindow(title,
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   width,
                                   height,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    assert(window);

    return window;
}

Window::Window(const size_t width, const size_t height) :
    _window {createWindow("", width, height), SDL_DestroyWindow},
    _gl {_window.get()}
{}

size_t Window::width() const {
    SDL_DisplayMode mode;
    SDL_GetWindowDisplayMode(_window.get(), &mode);
    return mode.w;
}

size_t Window::height() const {
    SDL_DisplayMode mode;
    SDL_GetWindowDisplayMode(_window.get(), &mode);
    return mode.h;
}

void Window::swapBuffers() {
#ifdef __WIN32__
    eglSwapBuffers(_gl.display, _gl.surface);
#else
    SDL_GL_SwapWindow(_window.get());
#endif // __WIN32__
}
