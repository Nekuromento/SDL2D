#include "LoadAtlas.hpp"

#include <type_traits>

#include "Core/Concurrency/Job.hpp"
#include "Core/Concurrency/JobQueue.hpp"
#include "Core/Memory/DoubleEndedLinearAllocator.hpp"
#include "Core/Memory/SmallObjectPool.hpp"
#include "Core/SpriteRegistry.hpp"
#include "Core/TextureRegistry.hpp"
#include "GFX/Sprite.hpp"
#include "GFX/Texture.hpp"
#include "Geom/Rect.hpp"
#include "Geom/Vector2D.hpp"
#include "IO/Stream.hpp"
#include "Png.hpp"
#include "Webp.hpp"

static const size_t hashSize = sizeof(uint32_t);
static const size_t hashAlignment = std::alignment_of<uint32_t>::value;

static const size_t textureSize = sizeof(Texture);
static const size_t textureAlignment = sizeof(Texture);

static const size_t spriteSize = sizeof(Sprite);
static const size_t spriteAlignment = sizeof(Sprite);

static void loadBlobPart(Stream& stream,
                         const Sprite* const image,
                         uint8_t* const buffer,
                         const Vector2D<size_t>& bufferSize,
                         DoubleEndedLinearAllocator& alloc) {
    const size_t chunkSize = stream.readIntLE();
    auto chunk =
        static_cast<uint8_t*>(alloc.allocateBack(chunkSize, 4, 0));
    stream.readTo(chunk, chunkSize);

    emplaceWebP(image->size, chunk, chunkSize, buffer, bufferSize, alloc);
}

static void loadBlob(Stream& stream,
                     const Sprite* const * const images,
                     uint8_t* const buffer,
                     const Vector2D<size_t>& size,
                     const size_t spriteCount,
                     DoubleEndedLinearAllocator& alloc) {
    for (size_t i = 0; i < spriteCount; ++i) {
        auto rewindPoint = alloc.rewindMarkerBack();
        loadBlobPart(stream, images[i], buffer, size, alloc);
        alloc.rewindBack(rewindPoint);
    }
}

static Sprite* loadSprite(Stream& stream,
                          const uint32_t texture,
                          DoubleEndedLinearAllocator& alloc) {
    const size_t left = stream.readShortLE();
    const size_t top = stream.readShortLE();
    const size_t width = stream.readShortLE();
    const size_t height = stream.readShortLE();
    const size_t dx = stream.readShortLE();
    const size_t dy = stream.readShortLE();

    assert(left <= 2048 && top <= 2048);
    assert(width <= 2048 && height <= 2048);
    assert(dx <= 2048 && dy <= 2048);

    const Vector2D<uint16_t> textureOffset(left, top);
    const Vector2D<uint16_t> size(left + width, top + height);
    const Vector2D<uint16_t> coordinateOffset(dx, dy);
    auto imageMemory = alloc.allocate(spriteSize, spriteAlignment, 0);
    return new (imageMemory) Sprite(texture, textureOffset, size, coordinateOffset);
}

static void loadSprites(Stream& stream,
                        Sprite** const images,
                        const uint32_t* const spriteHashes,
                        const size_t spriteCount,
                        const uint32_t texture,
                        DoubleEndedLinearAllocator &alloc) {
    for (size_t i = 0; i < spriteCount; ++i) {
        auto rewindPoint = alloc.rewindMarkerBack();
        images[i] = loadSprite(stream, texture, alloc);
        alloc.rewindBack(rewindPoint);

        SpriteRegistry::getDefault().registerResource(spriteHashes[i], images[i]);
    }
}

static uint8_t* readImageData(Stream& stream,
                              const uint32_t nameHash,
                              const uint32_t* const spriteHashes,
                              const size_t spriteCount,
                              const Vector2D<size_t>& size,
                              const Texture::Format format,
                              DoubleEndedLinearAllocator& alloc) {
    const bool isBlob = format == Texture::Uncompressed;
    const bool needAlpha = (format & Texture::Alpha) == Texture::Alpha;

    uint8_t* buffer = nullptr;
    if (isBlob) {
        buffer = static_cast<uint8_t*>(alloc.allocateBack(4 * size.x * size.y, 4, 0));
    } else {
        assert((format & Texture::Etc1) || (format & Texture::Pvrtc));
        const size_t textureSize = size.x * size.y / 2;
        const size_t bufferSize = needAlpha ? textureSize * 2 : textureSize;

        buffer = static_cast<uint8_t*>(alloc.allocateBack(bufferSize, 4, 0));

        stream.readTo(buffer, textureSize);
    }

    auto images = static_cast<Sprite**>(alloca(spriteCount * sizeof(Sprite*)));
    loadSprites(stream, images, spriteHashes, spriteCount, nameHash, alloc);

    if (isBlob) {
        loadBlob(stream, images, buffer, size, spriteCount, alloc);
    }

    return buffer;
}

static void readImageAlpha(Stream& stream, uint8_t* buffer, DoubleEndedLinearAllocator& alloc) {
    loadPng(stream, buffer, alloc);
}

static Texture* load(const uint32_t nameHash,
                     const char* const path,
                     DoubleEndedLinearAllocator& alloc) {
    Stream stream = Stream::fromFile(path, "rb");

    const size_t spriteCount = stream.readShortLE();
    const size_t spriteHashesBufferSize = hashSize * spriteCount;
    auto spriteHashes =
        static_cast<uint8_t*>(alloc.allocate(spriteHashesBufferSize, hashAlignment, 0));

    stream.readTo(spriteHashes, spriteHashesBufferSize);

    auto textureMemory = alloc.allocate(textureSize, textureAlignment, 0);
    auto texture =
        new (textureMemory) Texture(reinterpret_cast<uint32_t*>(spriteHashes), spriteCount);

    const size_t width = stream.readShortLE();
    const size_t height = stream.readShortLE();
    assert(width <= 2048 && height <= 2048);
    const Vector2D<size_t> size(width, height);

    const auto format = static_cast<Texture::Format>(stream.readByte());
    uint8_t* const buffer =
        readImageData(stream, nameHash, texture->sprites, spriteCount, size, format, alloc);

    Texture::upload(texture->handle, buffer, format, size);

    const bool needAlpha = (format & Texture::Alpha) == Texture::Alpha;
    if (needAlpha) {
        readImageAlpha(stream, buffer, alloc);
        Texture::upload(texture->handle, buffer, Texture::Alpha, size);
    }

    return texture;
}

struct Context {
    const uint32_t hash;
    const char* const path;
    DoubleEndedLinearAllocator& alloc;

    Context(const uint32_t hash, const char* const path, DoubleEndedLinearAllocator& alloc) :
        hash {hash},
        path {path},
        alloc(alloc)
    {}
};

static const size_t ContextSize = sizeof(Context);
static const size_t ContextAlignment = std::alignment_of<Context>::value;

static Context* setupContext(const uint32_t hash,
                             const char* const path,
                             DoubleEndedLinearAllocator& alloc) {
    void* memory =
        SmallObjectPool::getDefault().allocate(ContextSize, ContextAlignment, 0);
    return new (memory) Context(hash, path, alloc);
}

void releaseContext(const Context* const context) {
    SmallObjectPool::getDefault().free(const_cast<Context* const>(context));
}

void Loader::loadAtlas(const uint32_t hash,
                       const char* const path,
                       DoubleEndedLinearAllocator& alloc) {
    JobQueue::getDefault().add(Job([](void* payload) {
        const auto context = static_cast<Context*>(payload);

        auto rewindPoint = context->alloc.rewindMarkerBack();
        auto texture = load(context->hash, context->path, context->alloc);
        context->alloc.rewindBack(rewindPoint);

        TextureRegistry::getDefault().registerResource(context->hash, texture);

        releaseContext(context);
    }, setupContext(hash, path, alloc)));
}
