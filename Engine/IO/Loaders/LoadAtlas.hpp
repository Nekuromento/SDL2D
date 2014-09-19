#ifndef LoadAtlas_h__
#define LoadAtlas_h__

#include <cstdint>

class DoubleEndedLinearAllocator;

namespace Loader {

    void loadAtlas(const uint32_t hash, const char* const path, DoubleEndedLinearAllocator& alloc);

}

#endif // LoadAtlas_h__