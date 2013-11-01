#ifndef Webp_h__
#define Webp_h__

#include <cstdint>

class DoubleEndedLinearAllocator;

template <typename T>
struct Vector2D;
struct Sprite;

bool emplaceWebP(const Vector2D<uint16_t>& image,
                 const uint8_t* const chunk,
                 const size_t chunkSize,
                 uint8_t* const buffer,
                 const Vector2D<size_t>& bufferSize,
                 DoubleEndedLinearAllocator& alloc);

#endif // Webp_h__