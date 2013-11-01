#include "Stream.hpp"

#include <cassert>

#include "SDL_rwops.h"

#include "Core/String.hpp"
#include "Core/Memory/SmallObjectPool.hpp"

Stream::Stream(SDL_RWops* const source) :
    source {source}
{
    assert(source);
}

Stream::Stream(Stream&& other) :
    source {other.source}
{
    other.source = nullptr;
}

Stream::~Stream() {
    if (source) {
        SDL_RWclose(source);
        SmallObjectPool::getDefault().free(source);
    }
}

SDL_RWops* allocateSource() {
    const size_t size = sizeof(SDL_RWops);
    const size_t alignment = std::alignment_of<SDL_RWops>::value;

    return static_cast<SDL_RWops*>(SmallObjectPool::getDefault().allocate(size, alignment, 0));
}

Stream Stream::fromFP(FILE* const fp, const ClosePolicy policy) {
    return Stream(setupRWFromFP(allocateSource(), fp, policy == AutoClose ? SDL_TRUE : SDL_FALSE));
}

Stream Stream::fromFile(const char* filename, const char* mode) {
    return Stream(setupRWFromFile(allocateSource(), filename, mode));
}

Stream Stream::fromFile(const String& filename, const char* mode) {
    return Stream(setupRWFromFile(allocateSource(), filename.begin(), mode));
}

Stream Stream::fromMemory(uint8_t* const memory, const size_t size) {
    return Stream(setupRWFromMemory(allocateSource(), memory, size));
}

Stream Stream::fromMemory(const uint8_t* const memory, const size_t size) {
    return Stream(setupRWFromConstMemory(allocateSource(), memory, size));
}

bool Stream::done() const {
    return offset() == size();
}

size_t Stream::offset() const {
    return static_cast<size_t>(SDL_RWtell(source));
}

size_t Stream::size() const {
    return static_cast<size_t>(SDL_RWsize(source));
}

void Stream::skip(const size_t bytes) {
    SDL_RWseek(source, bytes, RW_SEEK_CUR);
}

void Stream::seek(const size_t position) {
    SDL_RWseek(source, position, RW_SEEK_SET);
}

uint8_t Stream::readByte() {
    return SDL_ReadU8(source);
}

uint16_t Stream::readShortLE() {
    return SDL_ReadLE16(source);
}

uint16_t Stream::readShortBE() {
    return SDL_ReadBE16(source);
}

float Stream::readFloatLE() {
    const uint32_t value = readIntLE();
    return *reinterpret_cast<const float*>(&value);
}

float Stream::readFloatBE() {
    const uint32_t value = readIntBE();
    return *reinterpret_cast<const float*>(&value);
}

uint32_t Stream::readIntLE() {
    return SDL_ReadLE32(source);
}

uint32_t Stream::readIntBE() {
    return SDL_ReadBE32(source);
}

double Stream::readDoubleLE() {
    const uint64_t value = readLongLE();
    return *reinterpret_cast<const double*>(&value);
}

double Stream::readDoubleBE() {
    const uint64_t value = readLongBE();
    return *reinterpret_cast<const double*>(&value);
}

uint64_t Stream::readLongLE() {
    return SDL_ReadLE64(source);
}

uint64_t Stream::readLongBE() {
    return SDL_ReadBE64(source);
}

String Stream::readString() {
    const size_t size = readShortLE();
    uint8_t* const sink = static_cast<uint8_t*>(alloca(size));
    readTo(sink, size);
    return String(reinterpret_cast<char* const>(sink), size);
}

void Stream::readTo(uint8_t* const sink, const size_t size) {
    SDL_RWread(source, sink, size, 1);
}

void Stream::readTo(Stream& sink, const size_t size) {
    // not the most efficient way of doing this
    for (size_t i = 0; i < size; ++i)
        sink.writeByte(readByte());
}

void Stream::writeByte(const uint8_t value) {
    SDL_WriteU8(source, value);
}

void Stream::writeShortLE(const uint16_t value) {
    SDL_WriteLE16(source, value);
}

void Stream::writeShortBE(const uint16_t value) {
    SDL_WriteBE16(source, value);
}

void Stream::writeFloatLE(const float value) {
    writeIntLE(*reinterpret_cast<const uint32_t*>(&value));
}

void Stream::writeFloatBE(const float value) {
    writeIntBE(*reinterpret_cast<const uint32_t*>(&value));
}

void Stream::writeIntLE(const uint32_t value) {
    SDL_WriteLE32(source, value);
}

void Stream::writeIntBE(const uint32_t value) {
    SDL_WriteBE32(source, value);
}

void Stream::writeDoubleLE(const double value) {
    writeLongLE(*reinterpret_cast<const uint64_t*>(&value));
}

void Stream::writeDoubleBE(const double value) {
    writeLongBE(*reinterpret_cast<const uint64_t*>(&value));
}

void Stream::writeLongLE(const uint64_t value) {
    SDL_WriteLE64(source, value);
}

void Stream::writeLongBE(const uint64_t value) {
    SDL_WriteBE64(source, value);
}

void Stream::writeString(const String& value) {
    writeShortLE(value.size());
    writeFrom(reinterpret_cast<const uint8_t*>(value.begin()), value.size());
}

void Stream::writeFrom(const uint8_t* const memory, const size_t size) {
    SDL_RWwrite(source, memory, size, 1);
}

void Stream::writeFrom(Stream& source, const size_t size) {
    source.readTo(*this, size);
}

void Stream::swap(Stream& other) {
    std::swap(source, other.source);
}
