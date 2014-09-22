#pragma once

#include "Core/String.hpp"

#include <cstddef>
#include <String>

class HMACBase {
protected:
    static const uint8_t iTranslationTable[];
    static const uint8_t oTranslationTable[];

    void translate(const void* const in, const size_t size, const uint8_t* const table, void* const result);

    HMACBase() {};
};

template<typename T>
class HMAC : public HMACBase {
    T hash;

public:
    // By default overload resolution always uses 'const uint8_t*' constructor
    // even for static arrays, so we fix that with a wrapper
    // with implicit constructor
    struct ConstByte {
        const uint8_t* const data;

        ConstByte(const uint8_t* const data) :
            data(data)
        {}
    };

    typedef typename T::Digest digest;

    digest operator() (ConstByte key, ConstByte message, const size_t keySize, const size_t messageSize) {
        uint8_t keyBlock[T::BlockSize] = {0},
            oKeyPad[T::BlockSize],
            iKeyPad[T::BlockSize],
            oMessage[T::BlockSize + T::Size];
        uint8_t* iMessage;

        if (keySize > T::BlockSize)        {
            const digest keyHash = hash(key.data, keySize);
            memcpy(keyBlock, keyHash.data, T::Size);
        } else {
            memcpy(keyBlock, key.data, keySize);
        }

        translate(keyBlock, T::BlockSize, oTranslationTable, oKeyPad);
        translate(keyBlock, T::BlockSize, iTranslationTable, iKeyPad);

        iMessage = (uint8_t*) alloca(T::BlockSize + messageSize);
        memcpy(iMessage, iKeyPad, T::BlockSize);
        memcpy(iMessage + T::BlockSize, message.data, messageSize);

        digest iHash = hash(iMessage, T::BlockSize + messageSize);

        memcpy(oMessage, oKeyPad, T::BlockSize);
        memcpy(oMessage + T::BlockSize, iHash.data, T::Size);

        digest oHash = hash(oMessage, T::BlockSize + T::Size);

        return oHash;
    }

    template<size_t M, size_t N>
    digest operator() (const uint8_t (&key)[M], const uint8_t (&data)[N], const size_t keyLength = M, const size_t length = N) {
        const uint8_t* keyBytes = key;
        const uint8_t* bytes = data;
        return operator ()(key, bytes, keyLength < M ? keyLength : M, length < N ? length : N);
    }

    template<size_t M>
    digest operator() (const char(&key)[M], ConstByte data, const size_t keyLength, const size_t length) {
        const uint8_t* keyBytes = key;
        return operator ()(key, data.data, keyLength < M ? keyLength : M, length);
    }

    template<size_t N>
    digest operator() (ConstByte key, const uint8_t (&data)[N], const size_t keyLength, const size_t length = N) {
        const uint8_t* bytes = data;
        return operator ()(key.data, bytes, keyLength, length < N ? length : N);
    }

    template<size_t M>
    digest operator() (const uint8_t (&key)[M], const String& data, const size_t keyLength = M) {
        const uint8_t* keyBytes = key;
        return operator ()(key, data.begin(), keyLength < M ? keyLength : M, data.size());
    }

    template<size_t N>
    digest operator() (const String& key, const uint8_t (&data)[N], const size_t length = N) {
        const uint8_t* bytes = data;
        return operator ()(key.begin(), bytes, key.size(), length < N ? length : N);
    }

    digest operator() (const String& key, const String& data) {
        return operator ()(key.begin(), data.begin(), key.size(), data.size());
    }

    digest operator() (ConstByte key, const String& data, const size_t keyLength) {
        return operator ()(key.data, data.begin(), keyLength, data.size());
    }

    digest operator() (const String& key, ConstByte data, const size_t length) {
        return operator ()(key.begin(), data.data, key.size(), length);
    }
};
