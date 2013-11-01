#ifndef DoubleEndedLinearAllocator_h__
#define DoubleEndedLinearAllocator_h__

#include <cstdint>

#include "Util/defines.hpp"
#include "Util/noncopyable.hpp"

class DoubleEndedLinearAllocator : public util::Noncopyable {
    uint8_t* _start;
    uint8_t* _end;
    uint8_t* _current;
    uint8_t* _currentBack;

public:
    typedef size_t RewindMarker;

    DoubleEndedLinearAllocator(void* const start, void* const end) NOEXCEPT;

    void* allocate(const size_t size, const size_t alignment, const size_t offset) NOEXCEPT;
    void* allocateBack(const size_t size, const size_t alignment, const size_t offset) NOEXCEPT;

    RewindMarker rewindMarker() const NOEXCEPT;
    RewindMarker rewindMarkerBack() const NOEXCEPT;
    void rewind(const RewindMarker marker) NOEXCEPT;
    void rewindBack(const RewindMarker marker) NOEXCEPT;

    void reset() NOEXCEPT;
};

#endif // DoubleEndedLinearAllocator_h__