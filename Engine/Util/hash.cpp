#include "hash.hpp"

#include "defines.hpp"

#ifndef __GNUC__
#  include <cstdlib>
#  define ROTL32(a, b) _rotl(a, b)
#else
#  define ROTL32(a, b) ((a) << (b) | (a) >> (32 - (b)))
#endif

REALLY_INLINE inline uint32_t fmix32 (uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
uint32_t util::hash(const void* key, const size_t size, const uint32_t seed) {
    const uint8_t * data = static_cast<const uint8_t*>(key);
    const int nblocks = size / 4;

    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    //----------
    // body

    const uint32_t * blocks = reinterpret_cast<const uint32_t*>(data + nblocks * 4);

    for (int i = -nblocks; i; ++i) {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1, 13); 
        h1 = h1 * 5 + 0xe6546b64;
    }

    //----------
    // tail

    const uint8_t * tail = static_cast<const uint8_t*>(data + nblocks * 4);

    uint32_t k1 = 0;

    switch (size & 3) {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
        k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= size;

    return fmix32(h1);
}
