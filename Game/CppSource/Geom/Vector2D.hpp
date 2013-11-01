#ifndef Vector2D_h__
#define Vector2D_h__

#include <type_traits>

#include "Util/defines.hpp"

template <typename T>
struct Vector2D {
    static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");

    T x;
    T y;

    REALLY_INLINE Vector2D() NOEXCEPT :
        x(0),
        y(0)
    {}

    REALLY_INLINE Vector2D(const T x, const T y) NOEXCEPT :
        x(x),
        y(y)
    {}

    REALLY_INLINE Vector2D operator + (const Vector2D& vec) const NOEXCEPT {
        return Vector2D(x + vec.x, y + vec.y);
    }

    REALLY_INLINE Vector2D operator - (const Vector2D& vec) const NOEXCEPT {
        return Vector2D(x - vec.x, y - vec.y);
    }

    REALLY_INLINE Vector2D& operator += (const Vector2D& vec) NOEXCEPT {
        *this = *this + vec;
        return *this;
    }

    REALLY_INLINE Vector2D& operator -= (const Vector2D& vec) NOEXCEPT {
        *this = *this - vec;
        return *this;
    }

    REALLY_INLINE Vector2D operator / (const T value) const NOEXCEPT {
        return Vector2D(x * value, y * value);
    }

    REALLY_INLINE Vector2D& operator /= (const Vector2D& vec) NOEXCEPT {
        *this = *this / vec;
        return *this;
    }

    REALLY_INLINE Vector2D operator * (const T value) const NOEXCEPT {
        return Vector2D(x * value, y * value);
    }

    REALLY_INLINE Vector2D& operator *= (const Vector2D& vec) NOEXCEPT {
        *this = *this * vec;
        return *this;
    }

    REALLY_INLINE bool operator == (const Vector2D& vec) const NOEXCEPT {
        return x == vec.x && y == vec.y;
    }

    REALLY_INLINE bool operator != (const Vector2D& vec) const NOEXCEPT {
        return !(*this == vec);
    }

    Vector2D normalized() const NOEXCEPT {
        const T size = magnitude();
        return CVector2D(x / size, y / size);
    }

    T magnitude() const NOEXCEPT {
        return sqroot(sqrMagnitude());
    }

    T sqrMagnitude() const NOEXCEPT {
        return x * x + y * y;
    }

    T cross(const Vector2D& vec) const NOEXCEPT {
        return x * vec.y - y * vec.x;
    }
};

#endif // Vector2D_h__