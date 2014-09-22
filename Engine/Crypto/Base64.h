#pragma once

#include <cstdlib>
#include <cassert>

#include "Core/String.hpp"
#include "Util/FixedArray.hpp"

class Base64 {
    void decode(util::FixedArray<uint8_t>* const result, const uint8_t* const data, const size_t length);

    template <typename Allocator>
    util::FixedArray<uint8_t> decode(Allocator& alloc, const uint8_t* const data, const size_t length) {
        assert(data);
        assert(length);
        if (length % 4 != 0)
            return util::FixedArray<uint8_t>();

        size_t outLength = length / 4 * 3;
        if (data[length - 1] == '=') {
            --outLength;
        }
        if (data[length - 2] == '=') {
            --outLength;
        }
        auto result = util::FixedArray<uint8_t>{ alloc, outLength };
        decode(&result, data, length);
        return result;
    }

public:
    // By default overload resolution always uses 'const uint8_t*' constructor
    // even for static arrays, so we fix that with a wrapper
    // with implicit constructor
    struct ConstByte {
        const uint8_t* const data;

        ConstByte(const uint8_t* const data) :
            data{ data }
        {}
    };

    String encode(ConstByte data, size_t length);

    template<size_t N>
    String encode(const uint8_t(&data)[N], size_t length = N) {
        const uint8_t* bytes = data;
        return encode(bytes, length < N ? length : N);
    }

    String encode(const String& data) {
        return encode(reinterpret_cast<const uint8_t*>(data.begin()), data.size());
    }

    String encode(const util::FixedArray<uint8_t>& data) {
        return encode(&data[0], data.size());
    }

    template <typename Allocator>
    util::FixedArray<uint8_t> decode(Allocator& alloc, ConstByte data, const size_t length) {
        return decode(alloc, data.data, length);
    }

    template<typename Allocator, size_t N>
    util::FixedArray<uint8_t> decode(Allocator& alloc, const uint8_t(&data)[N], size_t length = N) {
        const uint8_t* bytes = data;
        return decode(alloc, bytes, length < N ? length : N);
    }

    template <typename Allocator>
    util::FixedArray<uint8_t> decode(Allocator& alloc, const String& data) {
        return decode(alloc, reinterpret_cast<const uint8_t*>(data.begin()), data.size());
    }

    template <typename Allocator>
    util::FixedArray<uint8_t> decode(Allocator& alloc, const util::FixedArray<uint8_t>& data) {
        return decode(alloc, &data[0], data.size());
    }
};
