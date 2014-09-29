#include "GFX/Animation/Sample.hpp"

Sample::Index Sample::peekIndex(const uint8_t* const sampleStream, const size_t playhead) {
    //TODO: endian-correctness
    return *reinterpret_cast<const Sample::Index*>(sampleStream + playhead);
}

namespace Transform {
    enum Type {
        Position = 0,
        Scale = 1,
        Rotation = 2,
        Shear = 4,
        Color = 8
    };
}

template <typename T>
REALLY_INLINE size_t readFromStream(const uint8_t* const stream, const size_t playhead, T* const target) {
    *target = *reinterpret_cast<const T*>(stream + playhead);

    return playhead + sizeof(T);
}

size_t Sample::read(const uint8_t* const sampleStream, const size_t playhead, Sample* const target) {
    const auto next = peekIndex(sampleStream, playhead);
    size_t newPlayhead = playhead + sizeof(Sample::Index);

    if (next.typeMask & Transform::Position)
        newPlayhead = readFromStream(sampleStream, newPlayhead, &target->position);

    if (next.typeMask & Transform::Scale)
        newPlayhead = readFromStream(sampleStream, newPlayhead, &target->scale);

    if (next.typeMask & Transform::Rotation)
        newPlayhead = readFromStream(sampleStream, newPlayhead, &target->rotation);

    if (next.typeMask & Transform::Shear)
        newPlayhead = readFromStream(sampleStream, newPlayhead, &target->shear);

    if (next.typeMask & Transform::Color)
        newPlayhead = readFromStream(sampleStream, newPlayhead, &target->color);

    return newPlayhead;
}

