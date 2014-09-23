#ifndef CapacityArray_h__
#define CapacityArray_h__

#include "noncopyable.hpp"

namespace util {

template <typename T>
class CapacityArray : public util::Noncopyable {
    T* const _ptr;
    size_t _capacity;
    size_t _size;

    static const size_t elementSize = sizeof(T);
    static const size_t alignment = std::alignment_of<T>::value;

    void destroy(T* const from) {
        auto to = end();
        for (auto pos = from; pos != to; ++pos)
            pos->~T();
    }

public:
    CapacityArray() :
        _ptr{ nullptr },
        _capacity{ 0 },
        _size{ 0 }
    {}

    template <typename Allocator>
    CapacityArray(Allocator& alloc, const size_t capacity) :
        _ptr{ static_cast<T*>(alloc.allocate(elementSize * capacity, alignment, 0)) },
        _capacity{ capacity },
        _size{ 0 }
    {}

    template <typename Allocator>
    CapacityArray(Allocator& alloc, const CapacityArray& other) :
        _ptr{ static_cast<T*>(alloc.allocate(elementSize * other._capacity, alignment, 0)) },
        _capacity{ other._capacity },
        _size{ other._size }
    {
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }

    CapacityArray(CapacityArray&& other) :
        _ptr{ other._ptr },
        _capacity{ other._capacity },
        _size{ other._size }
    {
        other._ptr = nullptr;
        other._capacity = 0;
        other._size = 0;
    }

    ~CapacityArray() {
        destroy(begin());
    }

    CapacityArray& operator =(CapacityArray&& other) NOEXCEPT {
        other.swap(*this);

        return *this;
    }

    void swap(CapacityArray& other) NOEXCEPT {
        std::swap(_ptr, other._ptr);
        std::swap(_capacity, other._capacity);
        std::swap(_size, other._size);
    }

    void clear() {
        destroy(begin());
        _size = 0;
    }

    void add(const T& handler) {
        assert(("Trying to add handler beyond event capacity", _size < _capacity));
        new (&_ptr[_size++]) T(handler);
    }

    void add(T&& handler) {
        assert(("Trying to add handler beyond event capacity", _size < _capacity));
        new (&_ptr[_size++]) T(std::move(handler));
    }

    void remove(const T& handler) {
        auto pos = std::remove(begin(), end(), handler);
        if (pos != end()) {
            destroy(pos);
            _size -= std::distance(pos, end());
        }
    }

    REALLY_INLINE T& operator[] (const size_t index) NOEXCEPT {
        assert(("Invalid index", index < _size));
        return _ptr[index];
    }

    REALLY_INLINE const T& operator[] (const size_t index) const NOEXCEPT {
        assert(("Invalid index", index < _size));
        return _ptr[index];
    }

    REALLY_INLINE T* begin() NOEXCEPT {
        return _ptr;
    }

    REALLY_INLINE const T* begin() const NOEXCEPT {
        return _ptr;
    }

    REALLY_INLINE T* end() NOEXCEPT {
        return _ptr + _size;
    }

    REALLY_INLINE const T* end() const NOEXCEPT {
        return _ptr + _size;
    }

    REALLY_INLINE bool empty() const NOEXCEPT {
        return _size == 0;
    }

    REALLY_INLINE size_t size() const NOEXCEPT {
        return _size;
    }
};

}

#endif // CapacityArray_h__
