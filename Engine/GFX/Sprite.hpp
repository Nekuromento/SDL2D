#ifndef Sprite_h__
#define Sprite_h__

#include <cstdint>

#include "Geom/Vector2D.hpp"

struct Sprite {
    typedef Vector2D<uint16_t> Vector;
    const Vector textureOffset;
    const Vector size;
    const Vector coordinateOffset;
    const uint32_t texture;

    Sprite(const uint32_t texture,
           const Vector& textureOffset,
           const Vector& size,
           const Vector& coordinateOffset);
};

#endif // Sprite_h__