#include "Crypto/Hex.h"

#include <cassert>

static const char* HEX_NUMBERS = "0123456789ABCDEF";

static size_t decodeHex(const char symbol){
    if (symbol >= '0' && symbol <= '9')
        return symbol - '0';

    if (symbol >= 'a' && symbol <='f')
        return symbol - 'a' + 10;

    if (symbol >= 'A' && symbol <= 'F')
        return symbol - 'A' + 10;

    assert(false);
    return 0;
}

void Hex::decode(const void* const input, const size_t length, char* const output) {
    const char* data = reinterpret_cast<const char*>(input);
    for (size_t i = 0; i < length; i += 2) {
        output[i / 2] = decodeHex(data[i]) << 4 | decodeHex(data[i + 1]);
    }
}

void Hex::encode(const void* const input, const size_t length, char* const output){
    const char* data = reinterpret_cast<const char*>(input);

    for (size_t i = 0, pos = 0; i < length; ++i) {
        output[pos++] = HEX_NUMBERS[(data[i] >> 4) & 0xf];
        output[pos++] = HEX_NUMBERS[(data[i] & 0xf)];
    }
}

void Hex::encode(const char ch, char* const output){
    output[0] = HEX_NUMBERS[(ch >> 4) & 0xf];
    output[1] = HEX_NUMBERS[(ch & 0xf)];
}