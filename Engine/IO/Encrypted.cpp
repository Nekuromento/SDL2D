#include "Encrypted.hpp"

#include "Core/String.hpp"
#include "Core/Memory/LinearAllocator.hpp"
#include "Core/Memory/SmallObjectPool.hpp"
#include "Crypto/RC4.h"

#include "SDL_rwops.h"

#include <cassert>
#include <cstdint>
#include <algorithm>

template<typename T>
static T* allocateSmallObject() {
    const size_t size = sizeof(T);
    const size_t alignment = std::alignment_of<T>::value;

    return static_cast<T*>(SmallObjectPool::getDefault().allocate(size, alignment, 0));
}

bool rc4_file_open(SDL_RWops* const rwops, const char* const filename, const char* const mode, const char* const key) {
    SDL_RWops* file = setupRWFromFile(allocateSmallObject<SDL_RWops>(), filename, mode);
    assert(file);

    rwops->hidden.unknown.data1 = file;
    //XXX: RC4 doesn't fit in default small object pool, should probably create a separate
    //     pool for them
    rwops->hidden.unknown.data2 = new (allocateSmallObject<RC4>()) RC4(reinterpret_cast<const uint8_t*>(key), strlen(key));

    return true;
}

static int64_t rc4_file_size(SDL_RWops* const context) {
    auto const rwops = reinterpret_cast<SDL_RWops*>(context->hidden.unknown.data1);
    return SDL_RWsize(rwops);
}

static int64_t rc4_file_seek(SDL_RWops* const context, int64_t offset, int whence) {
    auto const rwops = reinterpret_cast<SDL_RWops*>(context->hidden.unknown.data1);
    // if SDL_RWtell is called on this stream
    if (offset == 0 && whence == RW_SEEK_CUR)
        return SDL_RWtell(rwops);

    assert(("Unsupported", false));
    return 0;
}

static size_t rc4_file_read(SDL_RWops* const context, void *ptr, size_t size, size_t maxnum) {
    auto const rwops = reinterpret_cast<SDL_RWops*>(context->hidden.unknown.data1);
    auto const rc4 = reinterpret_cast<RC4*>(context->hidden.unknown.data2);

    const size_t readnum = SDL_RWread(rwops, ptr, size, maxnum);

    rc4->decryptInplace(reinterpret_cast<uint8_t*>(ptr), size * readnum);

    return readnum;
}

static size_t rc4_file_write(SDL_RWops* const context, const void *ptr, size_t size, size_t num) {
    auto const rwops = reinterpret_cast<SDL_RWops*>(context->hidden.unknown.data1);
    auto const rc4 = reinterpret_cast<RC4*>(context->hidden.unknown.data2);

    const size_t allocSize = size * num;
    auto mem = static_cast<uint8_t*>(alloca(allocSize));
    LinearAllocator alloc{ mem, mem + allocSize };
    const auto encrypted = rc4->encrypt(alloc, reinterpret_cast<const uint8_t*>(ptr), allocSize);

    return SDL_RWwrite(rwops, &encrypted[0], encrypted.size(), 1);
}

static int rc4_file_close(SDL_RWops* const context) {
    int status = 0;
    if (context) {
        auto const rwops = reinterpret_cast<SDL_RWops*>(context->hidden.unknown.data1);
        auto const rc4 = reinterpret_cast<RC4*>(context->hidden.unknown.data2);

        if (SDL_RWclose(rwops) != 0) {
            status = SDL_Error(SDL_EFWRITE);
        }

        SmallObjectPool::getDefault().free(rc4);
        SmallObjectPool::getDefault().free(rwops);
    }
    return status;
}

SDL_RWops* setupRWFromEncryptedFile(SDL_RWops* const rwops, const char* const filename, const char* const mode, const char* const key) {
    const bool opened = rc4_file_open(rwops, filename, mode, key);
    assert(opened);

    rwops->size = rc4_file_size;
    rwops->seek = rc4_file_seek;
    rwops->read = rc4_file_read;
    rwops->write = rc4_file_write;
    rwops->close = rc4_file_close;

    return rwops;
}
