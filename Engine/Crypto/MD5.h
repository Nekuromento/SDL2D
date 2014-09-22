#pragma once

#include <cstdlib>

class String;

struct MD5 {
    // By default overload resolution always uses 'const uint8_t*' constructor
    // even for static arrays, so we fix that with a wrapper
    // with implicit constructor
    struct ConstByte {
        const uint8_t* const data;

        ConstByte(const uint8_t* const data) :
            data{ data }
        {}
    };

    static const size_t Size = 16;
    static const size_t BlockSize = 64;

    struct Digest {
        uint8_t data[Size];

        bool operator== (const Digest& other);
        bool operator!= (const Digest& other);
        String toString();
    };

    Digest operator() (ConstByte data, const size_t length);

    template<size_t N>
    Digest operator() (const uint8_t (&data)[N], const size_t length = N) {
        const uint8_t* const bytes = data;
        return operator() (bytes, length < N ? length : N);
    }

    Digest operator() (const String& data);
};
