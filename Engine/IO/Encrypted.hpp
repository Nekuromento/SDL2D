#ifndef Encrypted_h__
#define Encrypted_h__

struct SDL_RWops;
class String;

SDL_RWops* setupRWFromEncryptedFile(SDL_RWops* const rwops, const char* const filename, const char* const mode, const char* const key);

#endif // Encrypted_h__
