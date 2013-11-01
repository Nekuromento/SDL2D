#ifndef CapacityArray_h__
#define CapacityArray_h__

#include "noncopyable.hpp"

namespace util {

template <typename T>
class CapacityArray : public util::Noncopyable {
    T* const ptr;
    const size_t capacity;
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
    CapacityArray(Allocator& alloc, const size_t capacity) :
        ptr {static_cast<T*>(alloc.allocate(elementSize * capacity, alignment, 0))},
        capacity {capacity},
        size {0}
    {}

    template <typename Allocator>
    CapacityArray(Allocator& alloc, const CapacityArray& other) :
        ptr {static_cast<T*>(alloc.allocate(elementSize * other.capacity, alignment, 0))},
        capacity {other.capacity},
        size {other.size}
    {
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }

    ~CapacityArray() {
        destroy(begin());
    }

    void clear() {
        destroy(begin());
        size = 0;
    }

    void add(const T& handler) {
        assert(("Trying to add handler beyond event capacity", size < capacity));
        new (&ptr[size++]) T(handler);
    }

    void add(T&& handler) {
        assert(("Trying to add handler beyond event capacity", size < capacity));
        new (&ptr[size++]) T(std::move(handler));
    }

    void remove(const T& handler) {
        auto pos = std::remove(begin(), end(), handler);
        if (pos != end()) {
            destroy(pos);
            size -= std::distance(pos, end());
        }
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

#endif // CapacityArray_h__