#ifndef LoadFont_h__
#define LoadFont_h__

#include <cstdint>

class DoubleEndedLinearAllocator;

namespace Loader {

    void loadFont(const uint32_t hash, const char* const path, DoubleEndedLinearAllocator& alloc);

}

#endif // LoadFont_h__