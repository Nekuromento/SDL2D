#include "Webp.hpp"

#include "webp/decode.h"

#include "Core/Memory/DoubleEndedLinearAllocator.hpp"
#include "Geom/Vector2D.hpp"
#include "GFX/Sprite.hpp"

bool emplaceWebP(const Vector2D<uint16_t>& bounds,
                 const uint8_t* const chunk,
                 const size_t chunkSize,
                 uint8_t* const buffer,
                 const Vector2D<size_t>& bufferSize,
                 DoubleEndedLinearAllocator& alloc) {
    WebPDecoderConfig config;
    WebPInitDecoderConfig(&config);

    config.options.alloc = [](const size_t size, void* const alloc) {
        return static_cast<DoubleEndedLinearAllocator*>(alloc)->allocateBack(size, 4, 0);
    };
    config.options.allocator = &alloc;

    const size_t offset = bufferSize.x * bounds.y + bounds.x;
    uint8_t* const imageStart = buffer + 4 * offset;
    config.output.is_external_memory = 1;
    config.output.colorspace = MODE_RGBA;
    config.output.u.RGBA.rgba = imageStart;
    config.output.u.RGBA.stride = 4 * bufferSize.x;
    config.output.u.RGBA.size = 4 * (bufferSize.x * bufferSize.y - offset);

    const VP8StatusCode decodeResult = WebPDecode(chunk, chunkSize, &config);

    return decodeResult == VP8_STATUS_OK;
}
