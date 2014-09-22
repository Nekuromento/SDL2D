#pragma once

#include <cstdlib>

namespace Hex {
    void decode(const void* const input, const size_t length, char* const output);
    void encode(const void* const input, const size_t length, char* const output);
    void encode(const char ch, char* const output);
};
