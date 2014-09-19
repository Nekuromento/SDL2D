#ifndef ResourcePack_h__
#define ResourcePack_h__

#include <cstdlib>

/******************************************************************************
 * Resource pack file format spec:
 *   -Header-
 *     Header contains lists of resources to load divided by category
 *     first 4 bytes are reserved for version information
 *     next 4 bytes are counters for each resource category
 *
 *     Next are resource sections for every resource type
 *
 *   -Resource section-
 *     Every resource section contains a section header of 2 bytes
 *     unique for each resource type and list of null terminated strings
 *     prepended by string 4 byte hash and string size with terminator included
 *     representing relative paths to resources
 *****************************************************************************/

class Stream;
class DoubleEndedLinearAllocator;

struct ResourcePack {
    uint32_t* atlases;
    uint32_t* animations;
    uint32_t* sounds;
    uint32_t* fonts;
    size_t atlasCount;
    size_t animationCount;
    size_t soundCount;
    size_t fontCount;

    typedef size_t RewindMarker;
    RewindMarker rewindPoint;

    static ResourcePack load(Stream& stream, DoubleEndedLinearAllocator& alloc);
    static void release(ResourcePack& pack, DoubleEndedLinearAllocator& alloc);
};

#endif // ResourcePack_h__
