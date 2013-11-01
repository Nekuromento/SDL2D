#ifndef Registry_h__
#define Registry_h__

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include "defines.hpp"

namespace util {

template <typename T, size_t Size>
class Registry {
    struct Entry {
        uint32_t nameHash;
        const T* texture;

        bool operator <(const Entry& other) const {
            return nameHash < other.nameHash;
        }
    };

    static const size_t MaxResourceCount = Size;
    static const size_t EntryStorageSize = MaxResourceCount * sizeof(Entry);
    static const size_t EntryAlignment = std::alignment_of<Entry>::value;

    Entry* const _entries;
    size_t _entryCount;

    Entry* entryForHash(const size_t nameHash) const NOEXCEPT {
        Entry* const begin = _entries;
        Entry* const end = _entries + _entryCount;

        const Entry value = {nameHash, nullptr};
        return std::lower_bound(begin, end, value);
    }

public:
    struct DefaultInstance {
        static Registry* defaultInstance;

        template <typename Allocator>
        DefaultInstance(Allocator& alloc) {
            assert(("Trying to initialize default instance twice", !defaultInstance));
            void* const memory =
                alloc.allocate(sizeof(Registry), std::alignment_of<Registry>::value, 0);
            defaultInstance = new (memory) Registry(alloc);
        }

        ~DefaultInstance() {
            assert(("Default instance already destroyed", defaultInstance));
            defaultInstance->~Registry();
            defaultInstance = nullptr;
        }
    };

    static Registry& getDefault() {
        return *DefaultInstance::defaultInstance;
    }

    template <typename Allocator>
    Registry(Allocator& alloc) NOEXCEPT :
        _entries {static_cast<Entry*>(alloc.allocate(EntryStorageSize, EntryAlignment, 0))},
        _entryCount {0}
    {}

    void registerResource(const uint32_t nameHash, const T* resource) NOEXCEPT {
        assert(("No resource", resource));
        assert(("Maximum resource count reached", _entryCount < MaxResourceCount));

        auto pos = entryForHash(nameHash);
        assert(("Resource name collision", pos == _entries + _entryCount || pos->nameHash != nameHash));

        auto end = _entries + _entryCount;
        std::rotate(pos, end, end + 1);

        const Entry value = {nameHash, resource};
        *pos = value;

        ++_entryCount;
    }

    void unregisterResource(const uint32_t nameHash) NOEXCEPT {
        auto pos = entryForHash(nameHash);
        assert(("No resource found", pos != _entries + _entryCount));

        std::rotate(pos, pos + 1, _entries + _entryCount);
        --_entryCount;
    }

    const T* resourceForHandle(const uint32_t nameHash) const NOEXCEPT {
        auto entry = entryForHash(nameHash);
        assert(("No resource found", entry != _entries + _entryCount));

        return entry->texture;
    }
};

template <typename T, size_t Size>
Registry<T, Size>* Registry<T, Size>::DefaultInstance::defaultInstance;

}

#endif // Registry_h__