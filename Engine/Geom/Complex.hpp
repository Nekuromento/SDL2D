#ifndef Complex_h__
#define Complex_h__

#include <type_traits>

#include "Util/defines.hpp"

template <typename T>
struct Complex {
    static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");

    T real;
    T imag;

    REALLY_INLINE Complex() NOEXCEPT
    {}

    REALLY_INLINE Complex(const T angle) NOEXCEPT :
        real {cosine(angle)},
        imag {sine(angle)}
    {}

    REALLY_INLINE Complex(const T real, const T imag) NOEXCEPT :
        real {real},
        imag {imag}
    {}

    REALLY_INLINE Complex& operator* (const Complex& other) const NOEXCEPT {
        return Complex(real * other.real - imag * other.imag,
                       imag * other.real + real * other.imag);
    }
};

template <typename T>
REALLY_INLINE static Complex<T> lerp(const Complex<T>& lhs, const Complex<T>& rhs, const float delta) {
    return Complex<T>(lhs.real * (1 - delta) + rhs.real * delta,
                      lhs.imag * (1 - delta) + rhs.imag * delta);
}

#endif // Complex_h__
