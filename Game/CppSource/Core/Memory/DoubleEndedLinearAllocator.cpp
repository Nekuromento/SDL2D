#include "DoubleEndedLinearAllocator.hpp"

#include <cassert>
#include <cstring>

#include "Util/ptr_util.hpp"

DoubleEndedLinearAllocator::DoubleEndedLinearAllocator(void* const start, void* const end) NOEXCEPT :
    _start {static_cast<uint8_t*>(start)},
    _end {static_cast<uint8_t*>(end)},
    _current {_start},
    _currentBack {_end}
{
    assert(start <= end);
}

void* DoubleEndedLinearAllocator::allocate(const size_t size, const size_t alignment, const size_t offset) NOEXCEPT {
    uint8_t* const aligned =
        util::alignUp(_current + offset, alignment) - offset;
    assert(aligned + size < _currentBack);
    _current = aligned + size;
    return aligned;
}

void* DoubleEndedLinearAllocator::allocateBack(const size_t size, const size_t alignment, const size_t offset) NOEXCEPT {
    uint8_t* const aligned =
        util::alignDown(_currentBack - size, alignment) - offset;
    assert(aligned > _current);
    _currentBack = aligned;
    return aligned;
}

void DoubleEndedLinearAllocator::reset() NOEXCEPT {
#if !defined(NDEBUG) && !defined(_NDEBUG)
    memset(_start, 0xDE, _end - _start);
#endif

    _current = _start;
    _currentBack = _end;
}

DoubleEndedLinearAllocator::RewindMarker DoubleEndedLinearAllocator::rewindMarker() const NOEXCEPT {
    return reinterpret_cast<RewindMarker>(_current);
}

DoubleEndedLinearAllocator::RewindMarker DoubleEndedLinearAllocator::rewindMarkerBack() const NOEXCEPT {
    return reinterpret_cast<RewindMarker>(_currentBack);
}

void DoubleEndedLinearAllocator::rewind(const RewindMarker marker) NOEXCEPT {
    uint8_t* const rewindPoint = reinterpret_cast<uint8_t*>(marker);
    assert(_start <= rewindPoint && rewindPoint <= _current);

#if !defined(NDEBUG) && !defined(_NDEBUG)
    memset(rewindPoint, 0xDE, _current - rewindPoint);
#endif

    _current = rewindPoint;
}

void DoubleEndedLinearAllocator::rewindBack(const RewindMarker marker) NOEXCEPT {
    uint8_t* const rewindPoint = reinterpret_cast<uint8_t*>(marker);
    assert(_currentBack <= rewindPoint && rewindPoint <= _end);

#if !defined(NDEBUG) && !defined(_NDEBUG)
    memset(_currentBack, 0xDE, rewindPoint - _currentBack);
#endif

    _currentBack = rewindPoint;
}
