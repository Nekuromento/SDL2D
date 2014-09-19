#ifndef hash_h__
#define hash_h__

#include <cstdlib>

namespace util {

    // Implements Murmurhash3
    uint32_t hash(const void* data, const size_t size, const uint32_t seed);

}

#endif // hash_h__
