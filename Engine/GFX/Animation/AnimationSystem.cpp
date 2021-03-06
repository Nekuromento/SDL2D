#include "GFX/Animation/AnimationSystem.hpp"

#include "Core/ClipRegistry.hpp"
#include "GFX/Color.hpp"
#include "Geom/Matrix2D.hpp"
#include "Geom/Vector2D.hpp"

AnimationSystem* AnimationSystem::DefaultInstance::defaultInstance;

struct Animation {
    uint32_t nameHash;
    const uint8_t* sampleStream;
    uint16_t streamLength;
};

typedef Matrix2D<float> Matrix;

template <typename T>
REALLY_INLINE size_t readFromStream(const uint8_t* const stream, const size_t playhead, T* const target) {
    //TODO: endian-correctness
    *target = *reinterpret_cast<const T*>(stream + playhead);

    return playhead + sizeof(T);
}

uint32_t advanceTime(AnimationSystem::AnimationState* const state,
                     const uint16_t time,
                     const uint8_t* const sampleStream,
                     const uint32_t playhead) {
    uint32_t newPlayhead = playhead;

    for (;;) {
        const Sample::Index next = Sample::peekIndex(sampleStream, newPlayhead);
        auto& sample = state[next.index];
        if (time < sample.timeEnd)
            return newPlayhead;

        sample.timeStart = sample.timeEnd;
        sample.before = sample.after;

        newPlayhead = Sample::read(sampleStream, newPlayhead, &sample.after);
    }

    return newPlayhead;
}


AnimationSystem::Handle AnimationSystem::startAnimation(const uint32_t clipHash,
                                                        const uint32_t animationHash,
                                                        const bool cycle) {
    assert(("Maximum animation count reached", clipCount < MaxPlayingClipCount));

    // We are swapping element at last playing position
    // and last paused position
    //
    // [rrrrrrrrr[p]pppppp[.].....]
    //          ^        ^
    //          |        --clipCount
    //          --playingClipCount
    //
    // [rrrrrrrrr[.]pppppp[p].....]
    //            ^        ^
    //            |        --clipCount
    //            --playingClipCount
    //
    // and initializing a new animation in the empty space
    lookup[handleIndeces[clipCount]] = lookup[handleIndeces[playingClipCount]];
    clips[clipCount] = clips[playingClipCount];

    const Handle oldHandle = lookup[firstFreeIndex];
    const Handle newHandle = { playingClipCount, static_cast<uint16_t>(oldHandle.cycle + 1) };

    AnimationState* nodes = nodeStates + (firstFreeIndex * MaxClipNodeCount);
    lookup[firstFreeIndex] = newHandle;

    handleIndeces[playingClipCount] = firstFreeIndex;
    firstFreeIndex = oldHandle.index;

    const Clip* prototype = ClipRegistry::getDefault().resourceForHandle(clipHash);
    assert(("Invalid clip handle", prototype));

    const Animation* animation = std::lower_bound(prototype->animations,
                                                  prototype->animations + prototype->animationCount,
                                                  animationHash,
                                                  [](const Animation& animation, const uint32_t hash){
                                                      return animation.nameHash < hash;
                                                  });
    assert(("Invalid animation handle", animation != prototype->animations + prototype->animationCount));
    assert(("Invalid animation handle", animation->nameHash == animationHash));

    auto clip = PlayingClip{ nodes,
                             prototype,
                             animation->sampleStream + sizeof(float),
                             0,
                             0,
                             0 };
    readFromStream(animation->sampleStream, 0, &clip.timeScale);

    clip.playhead = ::advanceTime(clip.nodeStates,
                                  clip.time,
                                  clip.sampleStream,
                                  clip.playhead);

    clips[playingClipCount] = clip;

    ++playingClipCount;
    ++clipCount;

    return newHandle;
}

void AnimationSystem::stopAnimation(const AnimationSystem::Handle handle) {
    const auto storedHandle = lookup[handle.index];
    assert(("Invalid animation handle", storedHandle.index < clipCount));
    assert(("Using handle of destroyed animation", storedHandle.cycle == handle.cycle));

    --playingClipCount;
    --clipCount;

    if (storedHandle.index <= playingClipCount) {
        // If it were playing we have to perform two swaps
        // First we swap with last playing element
        //
        // [rrr[S]rrrr[r]ppppppp......]
        //             ^       ^
        //             |       --clipCount
        //             --playingClipCount
        //
        // [rrr[r]rrrr[S]ppppppp......]
        //             ^       ^
        //             |       --clipCount
        //             --playingClipCount
        std::swap(clips[storedHandle.index], clips[playingClipCount]);
        std::swap(handleIndeces[storedHandle.index], handleIndeces[playingClipCount]);
        std::swap(lookup[handleIndeces[storedHandle.index]], lookup[handleIndeces[playingClipCount]]);

        // Then swap with the last paused element moves the animation to the end, where
        // its destroyed
        //
        // [rrrrrrrr[S]pppppp[p]......]
        //           ^        ^
        //           |        --clipCount
        //           --playingClipCount
        //
        // [rrrrrrrr[p]pppppp[S]......]
        //         ^        ^
        //         |        --clipCount
        //         --playingClipCount
        std::swap(clips[clipCount], clips[playingClipCount]);
        std::swap(handleIndeces[clipCount], handleIndeces[playingClipCount]);
        std::swap(lookup[handleIndeces[clipCount]], lookup[handleIndeces[playingClipCount]]);
    } else {
        // If the stopped animation was pausedd we can perform only one swap
        // with the last paused element
        //
        // [rrrrrrrrppp[S]pppp[p]......]
        //         ^           ^
        //         |           --clipCount
        //         --playingClipCount
        //
        // [rrrrrrrrppp[p]pppp[S]......]
        //         ^         ^
        //         |         --clipCount
        //         --playingClipCount
        std::swap(clips[clipCount], clips[storedHandle.index]);
        std::swap(handleIndeces[clipCount], handleIndeces[storedHandle.index]);
        std::swap(lookup[handleIndeces[clipCount]], lookup[handleIndeces[storedHandle.index]]);
   }

    lookup[handleIndeces[clipCount]].index = firstFreeIndex;
    firstFreeIndex = handleIndeces[clipCount];
}

bool AnimationSystem::pauseAnimation(const AnimationSystem::Handle handle) {
    const auto storedHandle = lookup[handle.index];
    assert(("Invalid animation handle", storedHandle.index < clipCount));
    assert(("Using handle of destroyed animation", storedHandle.cycle == handle.cycle));

    if (storedHandle.index >= playingClipCount)
        return false;

    --playingClipCount;

    // We are swapping with last playing element
    //
    // [rrr[P]rrrr[r]ppppppp......]
    //             ^       ^
    //             |       --clipCount
    //             --playingClipCount
    //
    // [rrr[r]rrrr[P]ppppppp......]
    //           ^         ^
    //           |         --clipCount
    //           --playingClipCount
    std::swap(clips[storedHandle.index], clips[playingClipCount]);
    std::swap(handleIndeces[storedHandle.index], handleIndeces[playingClipCount]);
    std::swap(lookup[handleIndeces[storedHandle.index]], lookup[handleIndeces[playingClipCount]]);

    return true;
}

bool AnimationSystem::resumeAnimation(const AnimationSystem::Handle handle) {
    const auto storedHandle = lookup[handle.index];
    assert(("Invalid animation handle", storedHandle.index < clipCount));
    assert(("Using handle of destroyed animation", storedHandle.cycle == handle.cycle));

    if (storedHandle.index < playingClipCount)
        return false;

    // We are swapping with first paused element
    //
    // [rrrrrrrrr[p]ppp[R]pp......]
    //          ^          ^
    //          |          --clipCount
    //           --playingClipCount
    //
    // [rrrrrrrrr[R]ppp[p]pp......]
    //            ^        ^
    //            |        --clipCount
    //             --playingClipCount
    std::swap(clips[storedHandle.index], clips[playingClipCount]);
    std::swap(handleIndeces[storedHandle.index], handleIndeces[playingClipCount]);
    std::swap(lookup[handleIndeces[storedHandle.index]], lookup[handleIndeces[playingClipCount]]);

    ++playingClipCount;

    return true;
}

void AnimationSystem::advanceTime(const float delta) {
    for (size_t i = 0; i < playingClipCount; ++i) {
        PlayingClip& clip = clips[i];

        clip.time += delta * clip.timeScale;
        clip.playhead = ::advanceTime(clip.nodeStates,
                                      clip.time,
                                      clip.sampleStream,
                                      clip.playhead);
    }
}

REALLY_INLINE static float progress(const uint16_t start, const uint16_t end, const uint8_t time) {
    const float whole = end - start;
    const float offset = time - start;
    return offset / whole;
}

void evaluateAnimation(const AnimationSystem::AnimationState* const state,
                       const uint8_t nodeCount,
                       Matrix* const localTransforms,
                       Color* const localColors,
                       const uint16_t time) {
    for (size_t i = 0; i < nodeCount; ++i) {
        const auto& frame = state[i];
        const float delta = progress(frame.timeStart, frame.timeEnd, time);

        //XXX: most of the changes are probably position, rotation and scale
        //     maybe we should process color and shear changes separately
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

