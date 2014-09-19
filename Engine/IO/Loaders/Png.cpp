#include "Png.hpp"

#include "png.h"

#include "Core/Memory/DoubleEndedLinearAllocator.hpp"
#include "IO/Stream.hpp"

static const size_t SignatureSize = 8;;

static bool validatePNGSignature(Stream& stream) {
    uint8_t signature[SignatureSize];

    stream.readTo(signature, SignatureSize);
    return png_check_sig(signature, SignatureSize) ? true : false;
}

static void readPNGData(png_structp readContext, png_bytep data, png_size_t length) {
    static_cast<Stream*>(png_get_io_ptr(readContext))->readTo(data, length);
}

bool loadPng(Stream& stream, uint8_t* const buffer, DoubleEndedLinearAllocator& alloc) {
    const bool isValid = validatePNGSignature(stream);
    if (!isValid)
        return false;

    png_structp readContext = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!readContext)
        return false;

    png_infop pngInfo = png_create_info_struct(readContext);
    if (!pngInfo) {
        png_destroy_read_struct(&readContext, 0, 0);
        return false;
    }

    png_infop pngInfoEnd = png_create_info_struct(readContext);
    if (!pngInfoEnd) {
        png_destroy_read_struct(&readContext, &pngInfo, 0);
        return false;
    }

    //PNG will jump here if some error occurs
    if (setjmp(png_jmpbuf(readContext))) {
        png_destroy_read_struct(&readContext, &pngInfo, &pngInfoEnd);
        return false;
    }

    //using custom read function to read from stream
    png_set_read_fn(readContext, &stream, &readPNGData);
    png_set_sig_bytes(readContext, SignatureSize);
    png_read_info(readContext, pngInfo);

    png_set_strip_16(readContext);
    png_set_expand_gray_1_2_4_to_8(readContext);
    png_set_tRNS_to_alpha(readContext);
    png_set_palette_to_rgb(readContext);

    png_read_update_info(readContext, pngInfo);

    const uint8_t bitsPerChannel = png_get_bit_depth(readContext, pngInfo);
    const uint8_t channelCount = png_get_channels(readContext, pngInfo);
    const size_t width = png_get_image_width(readContext, pngInfo);
    const size_t height = png_get_image_height(readContext, pngInfo);

    png_bytep* const rows =
        static_cast<png_bytep*>(alloc.allocateBack(sizeof(png_bytep) * height, 4, 0));

    const size_t stride = width * bitsPerChannel * channelCount / 8;
    for (size_t row = 0; row < height; ++row)
        rows[row] = buffer + row * stride;

    png_read_image(readContext, rows);
    png_read_end(readContext, pngInfo);
    png_destroy_read_struct(&readContext, &pngInfo, &pngInfoEnd);

    return true;
}
