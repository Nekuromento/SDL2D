#ifndef ClipRegistry_h__
#define ClipRegistry_h__

#include "Util/Registry.hpp"

static const size_t MaxClipCount = std::numeric_limits<uint16_t>::max();

struct Clip;

typedef util::Registry<Clip, MaxClipCount> ClipRegistry;

#endif /* ClipRegistry_h__ */
