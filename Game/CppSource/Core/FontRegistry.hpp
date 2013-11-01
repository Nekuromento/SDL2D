#ifndef FontRegistry_h__
#define FontRegistry_h__

#include "Util/Registry.hpp"

static const size_t MaxFontCount = 16;

struct Font;

typedef util::Registry<Font, MaxFontCount> FontRegistry;

#endif // FontRegistry_h__