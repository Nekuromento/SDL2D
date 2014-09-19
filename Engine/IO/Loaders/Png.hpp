#ifndef Png_h__
#define Png_h__

#include <cstdint>

class DoubleEndedLinearAllocator;
class Stream;

bool loadPng(Stream& stream, uint8_t* const buffer, DoubleEndedLinearAllocator& alloc);

#endif // Png_h__