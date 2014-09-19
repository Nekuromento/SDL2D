#include "LinearAllocator.hpp"

#include <cassert>
#include <cstring>

#include "Util/ptr_util.hpp"

LinearAllocator::LinearAllocator(void* const start, void* const end) NOEXCEPT :
    _start {static_cast<uint8_t*>(start)},
    _end {static_cast<uint8_t*>(end)},
    _current {_start}
{
    assert(start <= end);
}

void* LinearAllocator::allocate(const size_t size, const size_t alignment, const size_t offset) NOEXCEPT {
    uint8_t* const aligned =
        util::alignUp(_current + offset, alignment) - offset;
    assert(aligned + size < _end);
    _current = aligned + size;
    return aligned;
}

void LinearAllocator::reset() NOEXCEPT {
#if !defined(NDEBUG) && !defined(_NDEBUG)
    memset(_start, 0xDE, _end - _start);
#endif

    _current = _start;
}

LinearAllocator::RewindMarker LinearAllocator::rewindMarker() const NOEXCEPT {
    return reinterpret_cast<RewindMarker>(_current);
}

void LinearAllocator::rewind(const RewindMarker marker) NOEXCEPT {
    uint8_t* const rewindPoint = reinterpret_cast<uint8_t*>(marker);
    assert(_start <= rewindPoint && rewindPoint <= _current);

#if !defined(NDEBUG) && !defined(_NDEBUG)
    memset(rewindPoint, 0xDE, _current - rewindPoint);
#endif

    _current = rewindPoint;
}
