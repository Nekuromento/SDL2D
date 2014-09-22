#ifndef FixedArray_h__
#define FixedArray_h__

#include <algorithm>
#include <cstdlib>
#include <cassert>

#include "Util/defines.hpp"
#include "Util/noncopyable.hpp"

namespace util {

template <typename T>
class FixedArray : public util::Noncopyable {
    T* _ptr;
    size_t _size;

    static const size_t elementSize = sizeof(T);
    static const size_t alignment = std::alignment_of<T>::value;

    void destroy(T* const from) {
        auto to = end();
        for (auto pos = from; pos != to; ++pos)
            pos->~T();
    }

public:
    FixedArray() :
        _ptr{ nullptr },
        _size{ 0 }
    {}

    template <typename Allocator>
    FixedArray(Allocator& alloc, const size_t size) :
        _ptr{ static_cast<T*>(alloc.allocate(elementSize * size, alignment, 0)) },
        _size{ size }
    {
        std::uninitialized_fill(begin(), end(), T());
    }

    template <typename Allocator>
    FixedArray(Allocator& alloc, const FixedArray& other) :
        _ptr{ static_cast<T*>(alloc.allocate(elementSize * other.size(), alignment, 0)) },
        _size{ other.size() }
    {
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }

    FixedArray(FixedArray&& other) :
        _ptr{ other._ptr },
        _size{ other._size }
    {
        other._ptr = nullptr;
        other._size = 0;
    }

    ~FixedArray() {
        destroy(begin());
    }

    FixedArray& operator =(FixedArray&& other) NOEXCEPT {
        other.swap(*this);

        return *this;
    }

    void swap(FixedArray& other) {
        std::swap(_ptr, other._ptr);
        std::swap(_size, other._size);
    }

    REALLY_INLINE T& operator[] (const size_t index) NOEXCEPT {
        assert(("Invalid index", index < size()));
        return _ptr[index];
    }

    REALLY_INLINE const T& operator[] (const size_t index) const NOEXCEPT {
        assert(("Invalid index", index < size()));
        return _ptr[index];
    }

    REALLY_INLINE T* begin() NOEXCEPT {
        return _ptr;
    }

    REALLY_INLINE const T* begin() const NOEXCEPT {
        return _ptr;
    }

    REALLY_INLINE T* end() NOEXCEPT {
        return _ptr + size();
    }

    REALLY_INLINE const T* end() const NOEXCEPT {
        return _ptr + size();
    }

    REALLY_INLINE bool empty() const NOEXCEPT {
        return size() == 0;
    }

    REALLY_INLINE size_t size() const NOEXCEPT {
        return _size;
    }
};

}

#endif // FixedArray_h__
