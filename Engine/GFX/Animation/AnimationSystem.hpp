#ifndef AnimationSystem_h__
#define AnimationSystem_h__

#include <cassert>
#include <cstdlib>
#include <limits>
#include <new>

#include "Util/noncopyable.hpp"
#include "GFX/Animation/Sample.hpp"

struct Animation;

struct Clip {
    struct LeafNode {
        uint32_t nameHash;
        uint8_t parent;
    };

    const uint8_t* parents;
    const LeafNode* sprites;
    const LeafNode* clips;
    const Animation* animations;
    uint8_t nodeCount;
    uint8_t spriteCount;
    uint8_t clipCount;
    uint8_t animationCount;
};

class AnimationSystem : util::Noncopyable {
public:
    struct Handle {
        uint16_t index : 9;
        uint16_t cycle : 7;
    };

    struct AnimationState {
        uint16_t timeStart;
        uint16_t timeEnd;
        Sample before;
        Sample after;
    };

private:
    struct PlayingClip {
        AnimationState* nodeStates;
        const Clip* clip;
        const uint8_t* sampleStream;
        float timeScale;
        uint32_t playhead;
        uint16_t time;
    };

    static const size_t MaxPlayingClipCount = 16 * 1024;
    static const size_t MaxClipNodeCount = 256;

    static const size_t PlayingClipsStorageSize = MaxPlayingClipCount * sizeof(PlayingClip);
    static const size_t PlayingClipsAlignment = std::alignment_of<PlayingClip>::value;

    static const size_t HandleIndecesStorageSize = MaxPlayingClipCount * sizeof(uint16_t);
    static const size_t HandleIndecesAlignment = std::alignment_of<uint16_t>::value;

    static const size_t LookupStorageSize = MaxPlayingClipCount * sizeof(Handle);
    static const size_t LookupAlignment = std::alignment_of<Handle>::value;

    static const size_t NodeStatesStorageSize = MaxPlayingClipCount * MaxClipNodeCount * sizeof(AnimationState);
    static const size_t NodeStateAlignment = std::alignment_of<AnimationState>::value;

    // We store all clip instances in one array
    // All paused clips are moved to the end of array
    // This way we can process all running animations in one loop
    // [RRRRRRRRRppppppp......]
    //          ^      ^
    //          |      --clipCount
    //          --playingClipCount
    //
    // All animations are controled using a handle
    // Handles are indexes in a lookup table to decouple handle from the actual address of
    // the animation it represents
    //
    // To allow add/remove/pause/resume operations to be O(1) every running clip has its
    // handle index in lookup table. When the clip is moved index is used to update the
    // location handle will point to
    //
    // Handle contains an index into clip array and a cycle marker (used for detection of
    // invalid handles)
    //
    // Unused cells in lookup table are used to store index of the next free cell

    PlayingClip* clips;
    uint16_t* handleIndeces;
    AnimationState* nodeStates;
    Handle* lookup;
    uint16_t playingClipCount;
    uint16_t clipCount;
    uint16_t firstFreeIndex;

public:
    struct DefaultInstance {
        static AnimationSystem* defaultInstance;

        template <typename Allocator>
        DefaultInstance(Allocator& alloc) {
            assert(("Trying to initialize default instance twice", !defaultInstance));
            void* const memory =
                alloc.allocate(sizeof(AnimationSystem), std::alignment_of<AnimationSystem>::value, 0);
            defaultInstance = new (memory) AnimationSystem(alloc);
        }

        ~DefaultInstance() {
            assert(("Default instance already destroyed", defaultInstance));
            defaultInstance->~AnimationSystem();
            defaultInstance = nullptr;
        }
    };

    static AnimationSystem& getDefault() {
        return *DefaultInstance::defaultInstance;
    }

    template <typename Allocator>
    AnimationSystem(Allocator& alloc) :
        clips{ reinterpret_cast<PlayingClip*>(alloc.allocate(PlayingClipsStorageSize, PlayingClipsAlignment, 0)) },
        handleIndeces{ reinterpret_cast<uint16_t*>(alloc.allocate(HandleIndecesStorageSize, HandleIndecesAlignment, 0)) },
        nodeStates{ reinterpret_cast<AnimationState*>(alloc.allocate(NodeStatesStorageSize, NodeStateAlignment, 0)) },
        lookup{ reinterpret_cast<Handle*>(alloc.allocate(LookupStorageSize, LookupAlignment, 0)) },
        clipCount{ 0 },
        playingClipCount{ 0 },
        firstFreeIndex{ 0 }
    {
        // initialize lookup table with indeces of the next free cell
        for (size_t i = 0; i < MaxPlayingClipCount; ++i) {
            lookup[i] = { static_cast<uint16_t>(i + 1), 0 };
        }
    }

    Handle startAnimation(const uint32_t clip, const uint32_t animation, const bool cycle = false);
    void stopAnimation(const Handle handle);
    bool pauseAnimation(const Handle handle);
    bool resumeAnimation(const Handle handle);

    void advanceTime(const float delta);
    //evaluateAnimations
};

#endif /* AnimationSystem_h__ */
