#include "Crypto/RC4.h"

void RC4::init(const uint8_t* const key, const size_t length) {
    assert(key && length > 0 && length <= 256);

    uint8_t T[256];
    for (size_t i = 0; i < 256; ++i) {
        S[i] = i;
        T[i] = key[i % length];
    }

    for (size_t i = 0, j = 0; i < 256; ++i) {
        j = (j + S[i] + T[i]) & 0xFF;
        S[i] ^= S[j];
        S[j] ^= S[i];
        S[i] ^= S[j];
    }
}

void RC4::encryptRealImpl(const uint8_t* const in, uint8_t* const out, const size_t length) {
    for (size_t counter = 0; counter < length; ++counter) {
        i = (i + 1) & 0xFF;
        j = (j + S[i]) & 0xFF;
        S[i] ^= S[j];
        S[j] ^= S[i];
        S[i] ^= S[j];
        const size_t t = (S[i] + S[j]) & 0xFF;
        const size_t k = S[t];
        out[counter] = static_cast<uint8_t>(in[counter] ^ k);
    }
}

