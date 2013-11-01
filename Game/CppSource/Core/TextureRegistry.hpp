#ifndef TextureRegistry_h__
#define TextureRegistry_h__

#include "Util/Registry.hpp"

static const size_t MaxTextureCount = 64;

struct Texture;

typedef util::Registry<Texture, MaxTextureCount> TextureRegistry;

#endif // TextureRegistry_h__