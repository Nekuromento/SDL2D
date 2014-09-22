#pragma once

#include <cstdlib>
#include <cassert>

#include "Core/String.hpp"
#include "Util/FixedArray.hpp"

class String;

class RC4 {
    uint8_t S[256];
    size_t i;
    size_t j;

    void init(const uint8_t* const key, const size_t length);
    void encryptRealImpl(const uint8_t* const in, uint8_t* const out, const size_t length);

    template <typename Allocator>
    util::FixedArray<uint8_t> encryptImpl(Allocator& alloc, const uint8_t* const data, const size_t length) {
        assert(data);
        assert(length);

        auto result = util::FixedArray<uint8_t>{ alloc, length };
        encryptRealImpl(data, &result[0], length);
        return result;
    }

public:
    // By default overload resolution always uses 'const char*' constructor
    // even for static arrays, so we fix that with a wrapper
    // with implicit constructor
    struct ConstChar {
        const char* const data;

        ConstChar(const char* const data) :
            data{ data }
        {}
    };

    struct ConstByte {
        const uint8_t* const data;

        ConstByte(const uint8_t* const data) :
            data{ data }
        {}
    };

    RC4(ConstByte key, size_t length) :
        i{ 0 },
        j{ 0 }
    {
        init(key.data, length);
    }

    RC4(ConstChar key, size_t length) :
        i{ 0 },
        j{ 0 }
    {
        init(reinterpret_cast<const uint8_t*>(key.data), length);
    }

    template<size_t N>
    RC4(const uint8_t (&key)[N], size_t length = N) :
        i{ 0 },
        j{ 0 }
    {
        init(key, length < N ? length : N);
    }

    template<size_t N>
    RC4(const char (&key)[N], size_t length = N) :
        i{ 0 },
        j{ 0 }
    {
        init(reinterpret_cast<const uint8_t*>(key), length < N ? length : N);
    }

    RC4(const String& key) :
        i{ 0 },
        j{ 0 }
    {
        init(reinterpret_cast<const uint8_t*>(key.begin()), key.size());
    }
 
    void encryptInplace(uint8_t* const data, const size_t length) {
        assert(data);
        assert(length);

        encryptRealImpl(data, data, length);
    }

    void decryptInplace(uint8_t* const data, const size_t length) {
        assert(data);
        assert(length);

        // algorithm is symmetric
        encryptRealImpl(data, data, length);
    }

    template <typename Allocator>
    util::FixedArray<uint8_t> encrypt(Allocator& alloc, ConstByte data, const size_t length) {
        return encryptImpl(alloc, data.data, length);
    }

    template <typename Allocator, size_t N>
    util::FixedArray<uint8_t> encrypt(Allocator& alloc, const uint8_t(&data)[N], const size_t length = N) {
        return encryptImpl(alloc, data, length < N ? length : N);
    }

    template <typename Allocator>
    util::FixedArray<uint8_t> encrypt(Allocator& alloc, const String& data) {
        return encryptImpl(alloc, reinterpret_cast<const uint8_t*>(data.begin()), data.size());
    }

    template <typename Allocator>
    util::FixedArray<uint8_t> decrypt(Allocator& alloc, ConstByte data, size_t length) {
        // algorithm is symmetric
        return encryptImpl(alloc, data.data, length);
    }

    template <typename Allocator, size_t N>
    util::FixedArray<uint8_t> decrypt(Allocator& alloc, const uint8_t(&data)[N], size_t length = N) {
        // algorithm is symmetric
        return encryptImpl(alloc, data, length < N ? length : N);
    }

    template <typename Allocator>
    util::FixedArray<uint8_t> decrypt(Allocator& alloc, const String& data) {
        // algorithm is symmetric
        return encryptImpl(alloc, reinterpret_cast<const uint8_t*>(data.begin()), data.size());
    }
};
