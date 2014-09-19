#ifndef Stream_h__
#define Stream_h__

#include <cstdint>
#include <cstdio>

#include "Util/noncopyable.hpp"

class String;
struct SDL_RWops;

class Stream : public util::Noncopyable {
    SDL_RWops* source;

    Stream(SDL_RWops* const source);

public:
    enum ClosePolicy {
        AutoClose,
        NoClose
    };

    static Stream fromFP(FILE* const fp, const ClosePolicy policy);
    static Stream fromFile(const char* const filename, const char* const mode);
    static Stream fromFile(const String& filename, const char* const mode);
    static Stream fromCompressedFile(const char* const filename, const char* const mode);
    static Stream fromCompressedFile(const String& filename, const char* const mode);
    static Stream fromEncryptedFile(const char* const filename, const char* const mode, const char* const key);
    static Stream fromEncryptedFile(const String& filename, const char* const mode, const char* const key);
    static Stream fromMemory(uint8_t* const source, const size_t size);
    static Stream fromConstMemory(const uint8_t* const source, const size_t size);

    Stream(Stream&& other);
    ~Stream();

    Stream& operator =(Stream&& other);

    void swap(Stream& other);

    bool isValid() const;
    bool done() const;
    size_t offset() const;
    size_t size() const;

    size_t skip(const size_t bytes);
    size_t seek(const size_t position);

    uint8_t readByte();
    uint16_t readShortLE();
    uint16_t readShortBE();
    float readFloatLE();
    float readFloatBE();
    uint32_t readIntLE();
    uint32_t readIntBE();
    double readDoubleLE();
    double readDoubleBE();
    uint64_t readLongLE();
    uint64_t readLongBE();
    String readString();

    size_t readTo(uint8_t* const sink, const size_t size);
    size_t readTo(Stream& sink, const size_t size);

    void writeByte(const uint8_t value);
    void writeShortLE(const uint16_t value);
    void writeShortBE(const uint16_t value);
    void writeFloatLE(const float value);
    void writeFloatBE(const float value);
    void writeIntLE(const uint32_t value);
    void writeIntBE(const uint32_t value);
    void writeDoubleLE(const double value);
    void writeDoubleBE(const double value);
    void writeLongLE(const uint64_t value);
    void writeLongBE(const uint64_t value);
    void writeString(const String& value);

    size_t writeFrom(const uint8_t* const source, const size_t size);
    size_t writeFrom(Stream& source, const size_t size);
};

#endif // Stream_h__
