#ifndef Compressed_h__
#define Compressed_h__

struct SDL_RWops;
class String;

SDL_RWops* setupRWFromCompressedFile(SDL_RWops* const rwops, const char* const filename, const char* const mode);

#endif // Compressed_h__
