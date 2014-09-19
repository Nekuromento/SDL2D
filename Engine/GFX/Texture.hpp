#ifndef Texture_h__
#define Texture_h__

#include <cstdlib>

template <typename T>
struct Vector2D;

struct Texture {
    enum Format {
        Alpha = 0x1,
        Uncompressed = 0x10,
        Etc1 = 0x20,
        Pvrtc = 0x30
    };

    typedef uint32_t Handle;

    Handle handle;
    uint32_t* sprites;
    size_t spriteCount;

    Texture(uint32_t* const sprites, const size_t spriteCount);

    static void bind(const Handle handle);
    static void upload(const Handle handle,
                       const uint8_t* const buffer,
                       const Format format,
                       const Vector2D<size_t>& size);
};

#endif // Texture_h__
