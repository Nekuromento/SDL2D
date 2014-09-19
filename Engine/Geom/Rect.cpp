#include "Rect.hpp"

#include <algorithm>

bool Rect::hasInside(const int32_t x, const int32_t y) const NOEXCEPT {
    return this->x <= x &&
        right() > x && 
        this->y <= y &&
        bottom() > y;
}

Rect Rect::unionWith(const Rect& other) const NOEXCEPT {
    if (other.isEmpty())
        return *this;

    if (isEmpty())
        return other;

    const int32_t right = std::max(this->right(), other.right());
    const int32_t bottom = std::max(this->bottom(), other.bottom());
    const int32_t left = std::min(x, other.x);
    const int32_t top = std::min(y, other.y);
    const int32_t width = right - left;
    const int32_t height = bottom - top;

    return Rect(left, top, width, height);
}

Rect Rect::intersectWith(const Rect& other) const NOEXCEPT {
    if (other.isEmpty() || isEmpty())
        return Rect();

    const int32_t right = std::min(this->right(), other.right());
    const int32_t bottom = std::min(this->bottom(), other.bottom());
    const int32_t left = std::max(x, other.x);
    const int32_t top = std::max(y, other.y);
    const int32_t width = right - left;
    const int32_t height = bottom - top;

    return (width < 0 || height < 0) ? Rect() : Rect(left, top, width, height);
}
