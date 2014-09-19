#ifndef LinearAllocator_h__
#define LinearAllocator_h__

#include <cstdlib>

#include "Util/defines.hpp"
#include "Util/noncopyable.hpp"

class LinearAllocator : public util::Noncopyable {
    uint8_t* _start;
    uint8_t* _end;
    uint8_t* _current;

public:
    typedef size_t RewindMarker;

    LinearAllocator(void* const start, void* const end) NOEXCEPT;

    void* allocate(const size_t size, const size_t alignment, const size_t offset) NOEXCEPT;

    RewindMarker rewindMarker() const NOEXCEPT;
    void rewind(const RewindMarker marker) NOEXCEPT;

    void reset() NOEXCEPT;
};

#endif // LinearAllocator_h__
