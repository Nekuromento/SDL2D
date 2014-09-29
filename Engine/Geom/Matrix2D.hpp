#ifndef Matrix2D_h__
#define Matrix2D_h__

#include "Geom/Complex.hpp"
#include "Geom/Vector2D.hpp"

template <typename T>
struct Matrix2D {
    static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");

    T m[2][2];
    Vector2D<T> t;

    REALLY_INLINE Matrix2D& identify() NOEXCEPT {
        t.x = t.y = 0;
        m[0][0] = m[1][1] = 1;
        m[0][1] = m[1][0] = 0;

        return *this;
    }

    REALLY_INLINE Vector2D<T> transformVector(const Vector2D<T>& vec) const NOEXCEPT {
        return Vector2D<T>(m[0][0] * vec.x + m[1][0] * vec.y + t.x,
                           m[0][1] * vec.x + m[1][1] * vec.y + t.y);
    }

    REALLY_INLINE Vector2D<T> rotateVector(const Vector2D<T>& vec) const NOEXCEPT {
        return Vector2D<T>(m[0][0] * vec.x + m[1][0] * vec.y,
                           m[0][1] * vec.x + m[1][1] * vec.y);
    }

    REALLY_INLINE Matrix2D<T> operator* (const Matrix2D& other) const NOEXCEPT {
        Matrix2D result = multiplyRotation(other);

        result.t = other.transformVector(t);

        return result;
    }

    inline Matrix2D multiplyRotation(const Matrix2D& other) const NOEXCEPT {
        // explicitly using local variables to signal compiler that no aliasing is occurring
        const T a00 = m[0][0],
                a01 = m[0][1],
                a10 = m[1][0],
                a11 = m[1][1];
        const T b00 = other.m[0][0],
                b01 = other.m[0][1],
                b10 = other.m[1][0],
                b11 = other.m[1][1];

        Matrix2D result;
        result.m[0][0] = a00 * b00 + a01 * b10;
        result.m[0][1] = a00 * b01 + a01 * b11;
        result.m[1][0] = a10 * b00 + a11 * b10;
        result.m[1][1] = a10 * b01 + a11 * b11;

        return result;
    }

    REALLY_INLINE Matrix2D& operator*= (const Matrix2D& otherMatrix) NOEXCEPT {
        *this = *this * otherMatrix;
        return *this;
    }

    REALLY_INLINE void scale(const T scaleFactor) NOEXCEPT {
        scaleRotation(scaleFactor);
        scaleTranslation(scaleFactor);
    }

    REALLY_INLINE void scaleRotation(const T scaleFactor) NOEXCEPT {
        m[0][0] *= scaleFactor;
        m[0][1] *= scaleFactor;
        m[1][0] *= scaleFactor;
        m[1][1] *= scaleFactor;
    }

    REALLY_INLINE void scaleTranslation(const T scaleFactor) NOEXCEPT {
        t.x *= scaleFactor;
        t.y *= scaleFactor;
    }

    REALLY_INLINE void setRotation(const Complex<T>& rotation) NOEXCEPT {
        m[0][0] = m[1][1] = rotation.real;
        m[0][1] = rotation.imag;
        m[1][0] = -rotation.imag;
    }

    REALLY_INLINE void setShear(const Vector2D<T>& shear) NOEXCEPT {
        m[0][1] = shear.x;
        m[1][0] = shear.y;
    }

    REALLY_INLINE void setScale(const Vector2D<T>& scale) NOEXCEPT {
        m[0][0] = scale.x;
        m[1][1] = scale.y;
    }

    REALLY_INLINE void copyRotation(const Matrix2D& otherMatrix) NOEXCEPT {
        m[0][0] = otherMatrix.m[0][0];
        m[0][1] = otherMatrix.m[0][1];
        m[1][0] = otherMatrix.m[1][0];
        m[1][1] = otherMatrix.m[1][1];
    }

    REALLY_INLINE void setTranslation(const Vector2D<T>& translation) NOEXCEPT {
        t = translation;
    }

    REALLY_INLINE bool operator != (const Matrix2D& mat) const NOEXCEPT {
        return !isRotationSame(mat) || !isTranslationSame(mat);
    }

    REALLY_INLINE bool operator == (const Matrix2D& mat) const NOEXCEPT {
        return isRotationSame(mat) && isTranslationSame(mat);
    }

    bool isRotationSame(const Matrix2D& mat) const NOEXCEPT {
        return m[0][0] == mat.m[0][0] &&
            m[0][1] == mat.m[0][1] &&
            m[1][0] == mat.m[1][0] &&
            m[1][1] == mat.m[1][1];
    }

    bool isTranslationSame(const Matrix2D& mat) const NOEXCEPT {
        return t == mat.t;
    }

    Matrix2D inverse() const NOEXCEPT {
        const T determinant = m[0][0] * m[1][1] - m[0][1] * m[1][0];

        Matrix2D inverted;
        inverted.m[0][0] = m[1][1] / determinant;
        inverted.m[0][1] = -m[0][1] / determinant;
        inverted.m[1][0] = -m[1][0] / determinant;
        inverted.m[1][1] = m[0][0] / determinant;
        inverted.t.x = (m[0][1] * t.y - t.x * m[1][1]) / determinant;
        inverted.t.y = (t.x * m[1][0] - m[0][0] * t.y) / determinant;

        return inverted;
    }
};

#endif // Matrix2D_h__
