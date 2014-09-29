#ifndef Color_h__
#define Color_h__

#include <cstdint>

#include "Util/defines.hpp"

struct Color {
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
        uint32_t rgba;
    };

    REALLY_INLINE Color() NOEXCEPT :
        rgba {0}
    {
        a = 0xff;
    }

    REALLY_INLINE Color(uint32_t rgba) NOEXCEPT :
        rgba {rgba}
    {}

    REALLY_INLINE Color(const uint8_t r,
                        const uint8_t g,
                        const uint8_t b,
                        const uint8_t a) NOEXCEPT:
        r {r},
        g {g},
        b {b},
        a {a}
    {}

    Color operator* (Color color) NOEXCEPT {
        return Color(+r * color.r / 0xff,
                     +g * color.g / 0xff,
                     +b * color.b / 0xff,
                     +a * color.a / 0xff);
    }

    REALLY_INLINE bool operator== (Color color) const NOEXCEPT {
        return rgba == color.rgba;
    }
};

REALLY_INLINE static Color lerp(const Color& lhs, const Color& rhs, const float delta) {
    return Color(lhs.r * (1 - delta) + rhs.r * delta,
                 lhs.g * (1 - delta) + rhs.g * delta,
                 lhs.b * (1 - delta) + rhs.b * delta,
                 lhs.a * (1 - delta) + rhs.a * delta);
}

#endif // Color_h__
