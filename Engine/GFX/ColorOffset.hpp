#ifndef ColorOffset_h__
#define ColorOffset_h__

struct ColorOffset {
    int16_t r;
    int16_t g;
    int16_t b;
    int16_t a;

    ColorOffset() NOEXCEPT :
        r {0},
        g {0},
        b {0},
        a {0}
    {}

    ColorOffset(const int16_t r,
                const int16_t g,
                const int16_t b,
                const int16_t a) NOEXCEPT :
        r {r},
        g {g},
        b {b},
        a {a}
    {}

    bool operator== (const ColorOffset& other) const NOEXCEPT {
        return r == other.r &&
            g == other.g &&
            b == other.b &&
            a == other.a;
    }
};

#endif // ColorOffset_h__