#include "Compressed.hpp"

#include "Core/String.hpp"
#include "Core/Memory/SmallObjectPool.hpp"

#include "zlib.h"
#include "SDL_rwops.h"

#include <cassert>
#include <cstdint>
#include <algorithm>
#include <memory>

static SDL_RWops* allocateSource() {
    const size_t size = sizeof(SDL_RWops);
    const size_t alignment = std::alignment_of<SDL_RWops>::value;

    return static_cast<SDL_RWops*>(SmallObjectPool::getDefault().allocate(size, alignment, 0));
}

//XXX: gzip stores uncompressed stream size in last 4 bytes of file
//     if stream is larger then 4 Gb we are screwed
static size_t getFileSize(const char* const filename) {
    auto file = setupRWFromFile(allocateSource(), filename, "rb");

    SDL_RWseek(file, -4, SEEK_END);
    const size_t size = SDL_ReadLE32(file);
    SDL_RWclose(file);
    SmallObjectPool::getDefault().free(source);

    return size;
}

static bool zlib_file_open(SDL_RWops* const rwops, const char* const filename, const char* const mode) {
    auto fp = gzopen(filename, mode);
    if (!fp)
        return false;

    rwops->hidden.unknown.data1 = fp;
    rwops->hidden.unknown.data2 = reinterpret_cast<void*>(getFileSize(filename));

    return true;
}

static int64_t zlib_file_size(SDL_RWops* const context) {
    return reinterpret_cast<size_t>(context->hidden.unknown.data2);
}

static int64_t zlib_file_seek(SDL_RWops* const context, int64_t offset, int whence) {
    auto const fp = reinterpret_cast<gzFile>(context->hidden.unknown.data1);

    const auto pos = gzseek(fp, offset, whence);
    if (pos == -1)
        return SDL_Error(SDL_EFSEEK);
    return pos;
}

static size_t zlib_file_read(SDL_RWops* const context, void *ptr, size_t size, size_t maxnum) {
    auto const fp = reinterpret_cast<gzFile>(context->hidden.unknown.data1);

    const size_t nread = gzread(fp, ptr, size * maxnum);

    int error;
    gzerror(fp, &error);
    if (nread == 0 && error) {
        SDL_Error(SDL_EFREAD);
    }
    return nread;
}

static size_t zlib_file_write(SDL_RWops* const context, const void *ptr, size_t size, size_t num) {
    auto const fp = reinterpret_cast<gzFile>(context->hidden.unknown.data1);

    const size_t nwrote = gzwrite(fp, ptr, size * num);

    int error;
    gzerror(fp, &error);
    if (nwrote == 0 && error) {
        SDL_Error(SDL_EFWRITE);
    }
    return (nwrote);
}

static int zlib_file_close(SDL_RWops * context) {
    int status = 0;
    if (context) {
        auto const fp = reinterpret_cast<gzFile>(context->hidden.unknown.data1);

        if (gzclose(fp) != 0) {
            status = SDL_Error(SDL_EFWRITE);
        }
    }
    return status;
}

SDL_RWops* setupRWFromCompressedFile(SDL_RWops* const rwops, const char* const filename, const char* const mode) {
    const bool opened = zlib_file_open(rwops, filename, mode);
    assert(opened);

    rwops->size = zlib_file_size;
    rwops->seek = zlib_file_seek;
    rwops->read = zlib_file_read;
    rwops->write = zlib_file_write;
    rwops->close = zlib_file_close;

    return rwops;
}
