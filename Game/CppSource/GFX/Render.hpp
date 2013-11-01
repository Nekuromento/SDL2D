#ifndef Render_h__
#define Render_h__

#include "SceneGraph.hpp"

class LinearAllocator;
template <typename Allocator>
class ScopeStack;

class Window;

struct Render : public util::Noncopyable {
    typedef ScopeStack<LinearAllocator> ScopeAlloc;

    template <typename Allocator>
    explicit Render(Allocator& alloc) :
        scene {alloc}
    {}

    void render(ScopeAlloc& alloc, const Window* window);

    SceneGraph scene;
};

#endif // Render_h__