#ifndef AnimationState_h__
#define AnimationState_h__

#include "GFX/Animation/Sample.hpp"
#include "Util/noncopyable.hpp"
#include "Util/FixedArray.hpp"

template<typename T>
class Matrix2D;
struct Color;

struct AnimationState : util::Noncopyable {
    struct State {
        uint16_t timeStart;
        uint16_t timeEnd;
        Sample before;
        Sample after;
    };

    typedef Matrix2D<float> Matrix;

    util::FixedArray<State> state;

    template <typename Allocator>
    AnimationState(Allocator& alloc, const size_t spriteCount) :
        state{ alloc, spriteCount }
    {}

    size_t advanceTime(const uint16_t delta, const uint8_t* const sampleStream, const size_t playhead);

    void evaluate(Matrix* const localTransforms, Color* const localColors, const uint16_t time) const;
};

#endif /* AnimationState_h__ */
