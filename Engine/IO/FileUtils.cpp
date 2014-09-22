#include "FileUtils.h"

#include "Core/String.hpp"
#include "Core/Memory/SmallObjectPool.hpp"

#include "SDL_filesystem.h"

#include <cassert>
#include <memory>

#ifdef __WIN32__
#  include <io.h>
#  include <sys/types.h>
#  include <sys/stat.h>

#  include "SDL_windows.h"
#  include "Strsafe.h"

#  define stat64 _stat64
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#  include <dirent.h>
#endif //__WIN32__

#ifdef __WIN32__
static const char DATA_PATH[] = "../data/";
#else
static const char DATA_PATH[] = "data/";
#endif //__WIN32__

String FileUtils::basePath() {
    static std::unique_ptr<char, decltype(&SDL_free)> path{ SDL_GetBasePath(), &SDL_free };
    return String(path.get());
}

String FileUtils::writablePath() {
#ifdef __WIN32__
    //HACK: to return a path that is actually writable we ensure that the folder exists
    if (!exists("../data-ram/")) {
        std::unique_ptr<WCHAR, decltype(&SDL_free)> dirname{ WIN_UTF8ToString("../data-ram/"), SDL_free };
        CreateDirectory(dirname.get(), nullptr);
    }
    return "../data-ram/";
#else
    static auto path = std::unique_ptr<char, decltype(&SDL_free)>{ SDL_GetPrefPath("madhat", "hero"), &SDL_free };
    return String(path.get());
#endif //__WIN32__
}

String FileUtils::dataPath(const char* const fileName) {
    return basePath() + DATA_PATH + fileName;
}

String FileUtils::dataPath(const String& fileName) {
    return basePath() + DATA_PATH + fileName;
}

String FileUtils::writableDataPath(const char* const fileName) {
    return writablePath() + fileName;
}

String FileUtils::writableDataPath(const String& fileName) {
    return writablePath() + fileName;
}

int64_t FileUtils::size(const String& fileName) {
    return size(fileName.begin());
}

int64_t FileUtils::size(const char* const fileName) {
    struct stat64 buf;
    if (stat64(fileName, &buf) != 0)
        return 0;

    return buf.st_size;
}

bool FileUtils::isDir(const String& fileName) {
    return isDir(fileName.begin());
}

bool FileUtils::isDir(const char* const fileName) {
    struct stat64 buf;
    if (stat64(fileName, &buf) != 0)
        return 0;

    return (buf.st_mode & S_IFDIR) == S_IFDIR;
}

bool FileUtils::exists(const String& fileName) {
    return exists(fileName.begin());
}

bool FileUtils::remove(const String& fileName) {
    return remove(fileName.begin());
}

bool FileUtils::rename(const String& sourceName, const char* const targetName, const bool replace) {
    return rename(sourceName.begin(), targetName, replace);
}

bool FileUtils::rename(const char* const sourceName, const String& targetName, const bool replace) {
    return rename(sourceName, targetName.begin(), replace);
}

bool FileUtils::rename(const String& sourceName, const String& targetName, const bool replace) {
    return rename(sourceName.begin(), targetName.begin(), replace);
}

bool FileUtils::exists(const char* const fileName) {
    return access(fileName, 0) == 0;
}

bool FileUtils::remove(const char* const fileName) {
    return ::remove(fileName) == 0;
}

bool FileUtils::rename(const char* const sourceName, const char* const targetName, const bool replace) {
    if (replace)
        remove(targetName);

    return ::rename(sourceName, targetName) == 0;
}

#ifdef __WIN32__
struct IterationState {
    WIN32_FIND_DATA ffd;
    std::unique_ptr<void, decltype(&FindClose)> dirHandle;
    FileUtils::File file;

    IterationState(const char* const dir) :
        dirHandle{ INVALID_HANDLE_VALUE, FindClose }
    {
        TCHAR szDir[MAX_PATH];

        std::unique_ptr<WCHAR, decltype(&SDL_free)> dirname{ WIN_UTF8ToString(dir), SDL_free };
        StringCchCopy(szDir, MAX_PATH, dirname.get());
        StringCchCat(szDir, MAX_PATH, TEXT("/*"));

        dirHandle.reset(FindFirstFile(szDir, &ffd));

        std::unique_ptr<char, decltype(&SDL_free)> filename{ WIN_StringToUTF8(ffd.cFileName), SDL_free };
        file = FileUtils::File(filename.get());
    }
};
#else
struct IterationState {
    std::unique_ptr<DIR, decltype(&closedir)> dirHandle;
    dirent* ent;
    FileUtils::File file;

    IterationState(const char* const dir) :
        dirHandle{ opendir(dir), closedir },
        ent{ readdir(dirHandle.get()) },
        file{ ent->d_name }
    {}
};
#endif

FileUtils::File::File()
{}

FileUtils::File::File(const String& name) :
    name{ name }
{}

FileUtils::File::File(const char* const name) :
    name{ name }
{}

FileUtils::DirIterator::DirIterator(IterationState* const state) :
    state{ state }
{}

FileUtils::DirIterator::~DirIterator() {
    delete state;
}

FileUtils::DirIterator::DirIterator(DirIterator&& other) :
    state{ other.state }
{
    other.state = nullptr;
}

FileUtils::DirIterator& FileUtils::DirIterator::operator =(DirIterator&& other) {
    if (&other != this) {
        other.swap(*this);
    }
    return *this;
}

void FileUtils::DirIterator::swap(DirIterator& other) {
    std::swap(state, other.state);
}

FileUtils::DirIterator& FileUtils::DirIterator::operator ++() {
#ifdef __WIN32__
    const bool reachedEnd = FindNextFile(state->dirHandle.get(), &state->ffd) == 0;
#else
    state->ent = readdir(state->dirHandle.get());
    const bool reachedEnd = !state->ent;
#endif

    if (reachedEnd) {
        SmallObjectPool::getDefault().free(state);
        state = nullptr;
    } else {
#ifdef __WIN32__
        std::unique_ptr<char, decltype(&SDL_free)> filename{ WIN_StringToUTF8(state->ffd.cFileName), SDL_free };
        state->file = File(filename.get());
#else
        state->file = File(state->ent->d_name);
#endif
    }

    return *this;
}

const FileUtils::File& FileUtils::DirIterator::operator *() const {
    return state->file;
}

const FileUtils::File* FileUtils::DirIterator::operator ->() const {
    return &state->file;
}

bool FileUtils::DirIterator::operator ==(const DirIterator& other) const {
    return state == other.state;
}

bool FileUtils::DirIterator::operator !=(const DirIterator& other) const {
    return !(operator ==(other));
}

FileUtils::DirIterator FileUtils::iterateDir(const String& dirName) {
    return iterateDir(dirName.begin());
}

template<typename T>
static T* allocateSmallObject() {
    const size_t size = sizeof(T);
    const size_t alignment = std::alignment_of<T>::value;

    return static_cast<T*>(SmallObjectPool::getDefault().allocate(size, alignment, 0));
}

FileUtils::DirIterator FileUtils::iterateDir(const char* const dirName) {
    auto iter = DirIterator(new (allocateSmallObject<IterationState>()) IterationState(dirName));
#ifdef __WIN32__
    if (iter.state->dirHandle.get() == INVALID_HANDLE_VALUE)
        return DirIterator(nullptr);
#else
    if (!iter.state->ent)
        return DirIterator(nullptr);
#endif
    return iter;
}

FileUtils::DirIterator FileUtils::dirEnd(nullptr);
