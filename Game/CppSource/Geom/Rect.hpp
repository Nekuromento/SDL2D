#ifndef Rect_h__
#define Rect_h__

#include <cstdint>

#include "Util/defines.hpp"

template <typename T>
struct Vector2D;

template <typename T>
struct Point;

struct Rect {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;

    Rect(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) NOEXCEPT :
        x {x},
        y {y},
        width {width},
        height {height}
    {}

    Rect() NOEXCEPT :
        x {0},
        y {0},
        width {0},
        height {0}
    {}

    bool isEmpty() const NOEXCEPT {
        return width == 0 && height == 0;
    }

    bool hasInside(const int32_t x, const int32_t y) const NOEXCEPT;

    template <typename T>
    bool hasInside(const Vector2D<T>& point) const NOEXCEPT {
        return hasInside(static_cast<int32_t>(point.x), static_cast<int32_t>(point.y));
    }

    Rect unionWith(const Rect& other) const NOEXCEPT;
    Rect intersectWith(const Rect& other) const NOEXCEPT;

    bool operator ==(const Rect& other) const NOEXCEPT {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    bool operator !=(const Rect& other) const NOEXCEPT {
        return x != other.x || y != other.y || width != other.width || height != other.height;
    }

    REALLY_INLINE int32_t right() const NOEXCEPT {
        return x + width;
    }

    REALLY_INLINE int32_t bottom() const NOEXCEPT {
        return y + height;
    }
};

#endif // Rect_h__