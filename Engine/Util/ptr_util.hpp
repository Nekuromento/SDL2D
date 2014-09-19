#ifndef __HPP_UTIL_PTR_UTIL
#define __HPP_UTIL_PTR_UTIL

#include <cassert>

namespace util {

template <typename T>
static inline T alignDown(T value, const size_t alignment) {
    assert(alignment);
    const size_t offset = alignment - 1;
    const size_t mask = ~offset;
    return (T) ((size_t) value & mask);
}

template <typename T>
static inline T alignUp(T value, const size_t alignment) {
    assert(alignment);
    const size_t offset = alignment - 1;
    const size_t mask = ~offset;
    return (T) (((size_t) value + offset) & mask);
} 

}
#endif // !__HPP_UTIL_PTR_UTIL