#include "Texture.hpp"

#include <cassert>

#include "SDL_opengles2.h"

#include "Geom/Vector2D.hpp"

Texture::Texture(uint32_t* const sprites, const size_t spriteCount) :
    handle {0},
    sprites {sprites},
    spriteCount {spriteCount}
{
    assert(("Texture with no sprites", sprites && spriteCount));
    glGenTextures(1, &handle);
    assert(!glGetError() && handle);

    bind(handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::bind(const Handle handle) {
    glBindTexture(GL_TEXTURE_2D, handle);
    assert(!glGetError());
}

void Texture::upload(const Handle handle,
                     const uint8_t* const buffer,
                     const Format format,
                     const Vector2D<size_t>& size) {
    bind(handle);

    if (format == Alpha || format == Uncompressed) {
        const auto components = (format == Alpha) ? GL_ALPHA : GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, components, size.x, size.y, 0, components, GL_UNSIGNED_BYTE, buffer);
    } else if (format == Etc1 || format == Pvrtc) {
        const size_t bufferSize = size.x * size.y / 2;
        const auto components =
            (format == Etc1)
                ? GL_ETC1_RGB8_OES
                : GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;

        glCompressedTexImage2D(GL_TEXTURE_2D, 0, components, size.x, size.y, 0, bufferSize, buffer);
    } else {
        assert(("Unknown texture format", false));
    }
    assert(!glGetError());
    glFlush();
}
