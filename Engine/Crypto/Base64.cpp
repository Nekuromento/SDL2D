/*******************************************************************************
 * Copyright (C) 2004-2008 René Nyffenegger
 *               2011      http://stackoverflow.com/a/6782480/379088
 *               2012-2013 Sergei Solozhentsev
 *               2013-2014 Max Klyga
 *
 * This source code is provided 'as-is', without any express or implied
 * warranty. In no event will the author be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this source code must not be misrepresented; you must not
 *    claim that you wrote the original source code. If you use this source code
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original source code.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * René Nyffenegger rene.nyffenegger@adp-gmbh.ch
 ******************************************************************************/

#include "Crypto/Base64.h"

#include <cctype>

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static char decoding_table[256];
static int mod_table[] = { 0, 2, 1 };

void build_decoding_table() {
    for (int i = 0; i < 64; ++i) {
        decoding_table[(unsigned char) base64_chars[i]] = i;
    }
}

class TablesInit{
public:
    TablesInit() {
        build_decoding_table();
    }
} __tablesInit;

static inline bool is_base64(unsigned char c) {
    return isalnum(c) || (c == '+') || (c == '/');
}

String Base64::encode(ConstByte data, size_t length) {
    assert(data.data);
    assert(length);

    const char* cursor = reinterpret_cast<const char*>(data.data);

    String ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    auto result = static_cast<char*>(alloca((length + 2) /3 * 4 + 1));
    int resultPos = 0;

    while (length--) {
        char_array_3[i++] = *(cursor++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; ++i) {
                result[resultPos++] = base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; ++j) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; ++j) {
            result[resultPos++] = base64_chars[char_array_4[j]];
        }

        while((i++ < 3)) {
            result[resultPos++] = '=';
        }
    }
    result[resultPos] = 0;

    auto res = String(result, resultPos);

    return res;
}

void Base64::decode(util::FixedArray<uint8_t>* const result, const uint8_t* const data, const size_t length) {
    assert(result);
    for (size_t i = 0, j = 0; i < length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
            + (sextet_b << 2 * 6)
            + (sextet_c << 1 * 6)
            + (sextet_d << 0 * 6);

        if (j < result->size())
            (*result)[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < result->size())
            (*result)[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < result->size())
            (*result)[j++] = (triple >> 0 * 8) & 0xFF;
    }
}

