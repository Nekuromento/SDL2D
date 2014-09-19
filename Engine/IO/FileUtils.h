#ifndef CFILEUTILS_H_
#define CFILEUTILS_H_

#include "Core/String.hpp"

struct IterationState;

namespace FileUtils
{
    int64_t size(const String& fileName);
    int64_t size(const char* const fileName);

    String basePath();
    String writablePath();

    String dataPath(const String& fileName);
    String dataPath(const char* const filename);

    String writableDataPath(const String& fileName);
    String writableDataPath(const char* const filename);

    bool exists(const String& fileName);
    bool exists(const char* const fileName);

    bool isDir(const String& fileName);
    bool isDir(const char* const fileName);

    bool remove(const String& fileName);
    bool remove(const char* const fileName);

    bool rename(const String& sourceName, const String& targetName, const bool replace = true);
    bool rename(const String& sourceName, const char* const targetName, const bool replace = true);
    bool rename(const char* const sourceName, const String& targetName, const bool replace = true);
    bool rename(const char* const sourceName, const char* const targetName, const bool replace = true);

    struct File {
        String name;

        File();
        File(const String& name);
        File(const char* const name);
    };

    struct DirIterator {
        DirIterator& operator ++();
        const File& operator *() const;
        const File* operator ->() const;

        DirIterator(IterationState* const state);
        ~DirIterator();

        DirIterator(DirIterator&& other);
        DirIterator& operator =(DirIterator&& other);
        void swap(DirIterator& other);

        bool operator ==(const DirIterator& other) const;
        bool operator !=(const DirIterator& other) const;

        IterationState* state;
    };

    DirIterator iterateDir(const String& dirName);
    DirIterator iterateDir(const char* const dirName);

    extern DirIterator dirEnd;
};

#endif /* CFILEUTILS_H_ */
