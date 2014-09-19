#include <cassert>
#include <cstdint>

#include "SDL_platform.h"

void* operator new(size_t) throw() {
    assert(("Using raw new is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

void* operator new[](size_t) throw() {
    assert(("Using raw new is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

void operator delete(void*) throw() {
    assert(("Using raw delete is bad and you should feel bad. Use allocators", false));
}

void operator delete[](void*) throw() {
    assert(("Using raw delete is bad and you should feel bad. Use allocators", false));
}

#ifndef __WIN32__
extern "C" {

void *malloc(size_t) {
    assert(("Using malloc is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

void * calloc(size_t, size_t) {
    assert(("Using calloc is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

void free(void*) {
    assert(("Using free is bad and you should feel bad. Use allocators", false));
}

void * realloc(void*, size_t) {
    assert(("Using realloc is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

int posix_memalign(void**, size_t, size_t) {
    assert(("Using posix_memalign is bad and you should feel bad. Use allocators", false));
    return 0;
}

/* The older *NIX interface for aligned allocations;
   it's formally substituted by posix_memalign and deprecated,
   so we do not expect it to cause cyclic dependency with C RTL. */
void * memalign(size_t, size_t) {
    assert(("Using memalign is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

/* valloc allocates memory aligned on a page boundary */
void * valloc(size_t) {
    assert(("Using valloc is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

/* pvalloc allocates smallest set of complete pages which can hold
   the requested number of bytes. Result is aligned on page boundary. */
void * pvalloc(size_t) {
    assert(("Using pvalloc is bad and you should feel bad. Use allocators", false));
    return nullptr;
}

}
#endif
