#ifndef FixedArray_h__
#define FixedArray_h__
#include "noncopyable.hpp"

namespace util {

template <typename T>
class FixedArray : public util::Noncopyable {
    T* const ptr;
    size_t size;

    static const size_t elementSize = sizeof(T);
    static const size_t alignment = std::alignment_of<T>::value;

    void destroy(T* const from) {
        auto to = end();
        for (auto pos = from; pos != to; ++pos)
            pos->~T();
    }

public:
    template <typename Allocator>
    FixedArray(Allocator& alloc, const size_t size) :
        ptr {static_cast<T*>(alloc.allocate(elementSize * size, alignment, 0))},
        size {size}
    {
        std::uninitialized_fill(begin(), end(), T());
    }

    template <typename Allocator>
    FixedArray(Allocator& alloc, const FixedArray& other) :
        ptr {static_cast<T*>(alloc.allocate(elementSize * other.size, alignment, 0))},
        size {other.size},
    {
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }

    ~FixedArray() {
        destroy(begin());
    }

    REALLY_INLINE T& operator[] (const size_t index) NOEXCEPT {
        assert(("Invalid index", index < size));
        return ptr[index];
    }

    REALLY_INLINE const T& operator[] (const size_t index) const NOEXCEPT {
        assert(("Invalid index", index < size));
        return ptr[index];
    }

    REALLY_INLINE T* begin() NOEXCEPT {
        return ptr;
    }

    REALLY_INLINE const T* begin() const NOEXCEPT {
        return ptr;
    }

    REALLY_INLINE T* end() NOEXCEPT {
        return ptr + size;
    }

    REALLY_INLINE const T* end() const NOEXCEPT {
        return ptr + size;
    }

    REALLY_INLINE size_t empty() const NOEXCEPT {
        return size == 0;
    }
};

}


#endif // FixedArray_h__