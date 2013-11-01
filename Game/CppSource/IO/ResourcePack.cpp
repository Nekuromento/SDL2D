#include "ResourcePack.hpp"

#include <algorithm>
#include <cassert>

#include "Core/Memory/DoubleEndedLinearAllocator.hpp"
#include "Loaders/LoadAtlas.hpp"
#include "Loaders/LoadAnimation.hpp"
#include "Loaders/LoadFont.hpp"
#include "Loaders/LoadSound.hpp"
#include "Stream.hpp"

struct ResourcePackDecoder {
    const uint8_t version[4];
    ResourcePack (*load)(Stream& stream, DoubleEndedLinearAllocator& alloc);
};

static const size_t hashSize = sizeof(uint32_t);
static const size_t hashAlignment = std::alignment_of<uint32_t>::value;

static const size_t resourceCountVerion0 = 4;

enum SectionHeader {
    Atlases = 0xCAFE,
    Animations = 0xBEEF,
    Sounds = 0xBAAD,
    Fonts = 0xF00D,
};

struct SectionLoader {
    const SectionHeader header;
    void (*load)(const uint32_t hash, const char* const path, DoubleEndedLinearAllocator& alloc);
};

static const SectionLoader sectionLoadersVersion0[resourceCountVerion0] = {
    {Atlases, &Loader::loadAtlas},
    {Animations, &Loader::loadAnimation},
    {Sounds, &Loader::loadSound},
    {Fonts, &Loader::loadFont}
};

static uint32_t* loadResourceSectionVersion0(const size_t sectionIndex,
                                             const size_t size,
                                             Stream& stream,
                                             DoubleEndedLinearAllocator& alloc) {
    assert(sectionIndex < resourceCountVerion0);
    uint32_t* const hashes =
        static_cast<uint32_t*>(alloc.allocate(size * hashSize, hashAlignment, 0));

    auto& loader = sectionLoadersVersion0[sectionIndex];
    const uint16_t actualHeader = stream.readShortLE();
    const uint16_t expectedHeader = loader.header;
    assert(("Invalid resource section header", actualHeader == expectedHeader));

    const size_t MaxPathSize = 1024;
    uint8_t path[MaxPathSize] = {0};
    for (size_t i = 0; i < size; ++i) {
        const uint32_t hash = stream.readIntLE();
        const uint16_t pathSize = stream.readShortLE();
        assert(pathSize < MaxPathSize);

        stream.readTo(path, pathSize);

        loader.load(hash, reinterpret_cast<char* const>(path), alloc);

        hashes[i] = hash;
    }
    return hashes;
}

static ResourcePack loadVersion0(Stream& stream, DoubleEndedLinearAllocator& alloc) {
    ResourcePack pack;
    pack.rewindPoint = alloc.rewindMarker();

    size_t* const sizes[resourceCountVerion0] = {
        &pack.atlasCount, &pack.animationCount, &pack.soundCount, &pack.fontCount
    };

    for (auto& size : sizes)
        *size = stream.readByte();

    uint32_t** const resources[resourceCountVerion0] = {
        &pack.atlases, &pack.animations, &pack.sounds, &pack.fonts
    };

    for (size_t i = 0; i < resourceCountVerion0; ++i)
        *resources[i] = loadResourceSectionVersion0(i, *sizes[i], stream, alloc);

    return pack;
}

static const ResourcePackDecoder decoders[] = {
    { {'R', 'E', 'S', 0}, &loadVersion0 }
};

ResourcePack ResourcePack::load(Stream& stream, DoubleEndedLinearAllocator& alloc) {
    uint8_t version[4];
    stream.readTo(version, 4);

    auto decoder = std::find_if(std::begin(decoders), std::end(decoders), [&](const ResourcePackDecoder& decoder){
        return std::equal(version, version + 4, decoder.version);
    });
    assert(("Unknown resource pack version", decoder != std::end(decoders)));
    return decoder->load(stream, alloc);
}

void ResourcePack::release(ResourcePack& pack, DoubleEndedLinearAllocator& alloc) {
    //TODO: unregister things from apropriate registries;
    alloc.rewind(pack.rewindPoint);
}
