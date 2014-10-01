#ifndef Sample_h__
#define Sample_h__

#include "Geom/Vector2D.hpp"
#include "Geom/Complex.hpp"
#include "GFX/Color.hpp"

struct Sample {
    struct Index {
        uint16_t index : 10;
        uint8_t typeMask : 6;
        uint16_t time;
    };

    typedef Vector2D<float> Vector;
    typedef Complex<float> ComplexN;

    Vector position;
    Vector scale;
    ComplexN rotation;
    Vector shear;
    Color color;

    //XXX: should it be possible to use IO/Streams?
    static Index peekIndex(const uint8_t* const sampleStream, const size_t playhead);
    static size_t read(const uint8_t* const sampleStream, const size_t playhead, Sample* const target);
};

#endif // Sample_h__
