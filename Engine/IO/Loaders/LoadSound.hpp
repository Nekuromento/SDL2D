#ifndef LoadSound_h__
#define LoadSound_h__

#include <cstdint>

class DoubleEndedLinearAllocator;

namespace Loader {

void loadSound(const uint32_t hash, const char* const path, DoubleEndedLinearAllocator& alloc);

}

#endif // LoadSound_h__