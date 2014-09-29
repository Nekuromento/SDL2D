#include "GFX/Animation/AnimationState.hpp"

#include "Geom/Matrix2D.hpp"
#include "Geom/Vector2D.hpp"
#include "GFX/Color.hpp"

size_t AnimationState::advanceTime(const uint16_t time,
                                   const uint8_t* const sampleStream,
                                   const size_t playhead) {
    size_t newPlayhead = playhead;

    for (;;) {
        const Sample::Index next = Sample::peekIndex(sampleStream, newPlayhead);
        State& sample = state[next.index];
        if (time < sample.timeEnd)
            return newPlayhead;

        sample.timeStart = sample.timeEnd;
        sample.before = sample.after;

        newPlayhead = Sample::read(sampleStream, newPlayhead, &sample.after);
    }

    return newPlayhead;
}

REALLY_INLINE static float progress(const uint16_t start, const uint16_t end, const uint8_t time) {
    const float whole = end - start;
    const float offset = time - start;
    return offset / whole;
}

void AnimationState::evaluate(AnimationState::Matrix* const localTransforms,
                              Color* const localColors,
                              const uint16_t time) const {
    for (size_t i = 0; i < state.size(); ++i) {
        const State& frame = state[i];
        const float delta = progress(frame.timeStart, frame.timeEnd, time);

        const auto position = lerp(frame.before.position, frame.after.position, delta);
        const auto scale    = lerp(frame.before.scale,    frame.after.scale,    delta);
        const auto rotation = lerp(frame.before.rotation, frame.after.rotation, delta);
        const auto shear    = lerp(frame.before.shear,    frame.after.shear,    delta);
        const auto color    = lerp(frame.before.color,    frame.after.color,    delta);

        Matrix shearScaleMatrix;
        shearScaleMatrix.setShear(shear);
        shearScaleMatrix.setScale(scale);

        Matrix rotateMatrix;
        rotateMatrix.setRotation(rotation);

        auto resultMatrix = shearScaleMatrix.multiplyRotation(rotateMatrix);
        resultMatrix.t = position;

        localTransforms[i] = resultMatrix;
        localColors[i] = color;
    }
}

