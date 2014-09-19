#ifndef SpriteRegistry_h__
#define SpriteRegistry_h__

#include "Util/Registry.hpp"

static const size_t MaxSpriteCount = 4096;

struct Sprite;

typedef util::Registry<Sprite, MaxSpriteCount> SpriteRegistry;

#endif // SpriteRegistry_h__