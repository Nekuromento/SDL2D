#ifndef Subclip_h__
#define Subclip_h__

#include "GFX/Animation/AnimationState.hpp"
#include "Util/noncopyable.hpp"

struct Subclip : util::Noncopyable {
    AnimationState state;
    const uint8_t* sampleStream;
    size_t playhead;
};

#endif /* Subclip_h__ */
