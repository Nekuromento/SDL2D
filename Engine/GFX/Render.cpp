#include "Render.hpp"

#include "SDL_opengles2.h"

#include "Window.hpp"

void setup(const size_t width, const size_t height) {
    glViewport(0, 0, width, height);

    glDisable(GL_DITHER);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);
}

void Render::render(ScopeAlloc& alloc, const Window* window) {
    // Some devices trash FBO if depth buffer is not cleared
    glClear(GL_DEPTH_BUFFER_BIT);

    //renderStarting.invoke();
    //
    //if (_currentSurface != NULL) {
    //    OffscreenSurface::makeDisplayCurrent();
    //    _currentSurface = NULL;
    //}

    setup(window->width(), window->height());

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.updateGlobalTransforms();

    //TODO: draw

    glFlush();
}
