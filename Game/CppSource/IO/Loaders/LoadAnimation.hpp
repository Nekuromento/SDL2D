#ifndef LoadAnimation_h__
#define LoadAnimation_h__

#include <cstdint>

class DoubleEndedLinearAllocator;

namespace Loader {

    void loadAnimation(const uint32_t hash, const char* const path, DoubleEndedLinearAllocator& alloc);

}

#endif // LoadAnimation_h__