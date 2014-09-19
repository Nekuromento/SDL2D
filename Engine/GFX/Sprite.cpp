#include "Sprite.hpp"

Sprite::Sprite(const uint32_t texture,
               const Vector& textureOffset,
               const Vector& size,
               const Vector& coordinateOffset) :
    textureOffset {textureOffset},
    size {size},
    coordinateOffset {coordinateOffset},
    texture {texture}
{
}
