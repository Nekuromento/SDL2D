#include "SmallObjectPool.hpp"

#include "Util/ptr_util.hpp"

SmallObjectPool* SmallObjectPool::DefaultInstance::defaultInstance;

SmallObjectPool::DefaultInstance::~DefaultInstance() {
    assert(("Default instance already destroyed", defaultInstance));
    defaultInstance->~SmallObjectPool();
    defaultInstance = nullptr;
}

SmallObjectPool& SmallObjectPool::getDefault() {
    return *DefaultInstance::defaultInstance;
}

struct FreeList {
    FreeList* next;
};

struct SmallObjectPoolHeader {
    uint8_t flags : 7;
    uint8_t allocated : 1;
    uint8_t size;
};

static const size_t HeaderSize = sizeof(SmallObjectPoolHeader);
static_assert(HeaderSize + sizeof(FreeList) <= SmallObjectPool::BinSize,
              "It should be possible to store header and a pointer to next free chunk inside smallest bin");

SmallObjectPool::~SmallObjectPool() {
    bool foundMemoryLeak = false;
    uint8_t* current = _memory;
    while (current < _current) {
        auto header = reinterpret_cast<SmallObjectPoolHeader*>(current);
        foundMemoryLeak |= header->allocated;
        //TODO: trace memory leak location
        current = current + header->size + HeaderSize;
    }
    assert(!foundMemoryLeak);
}

inline static void setupHeader(void* data, const size_t size) {
    auto header = reinterpret_cast<SmallObjectPoolHeader*>(data);
    header->flags = 0;
    header->allocated = true;
    header->size = size;
}

void* SmallObjectPool::allocate(const size_t size, const size_t alignment, const size_t offset) {
    assert(("Not enough memory", _size - (_current - _memory) >= size + HeaderSize));
    assert(("Trying to use small object pool for big objects", size <= MaxSmallObjectSize));
    assert(("Unsupported alignment", alignment == BinSize || alignment == 1));
    assert(("Offset must be a multiple of alignment", (offset & (alignment - 1)) == 0));

    const size_t binIndex = size >> BinSizeBitShift;
    FreeList* bin = _free[binIndex];
    if (bin) {
        _free[binIndex] = bin->next;
        auto header = reinterpret_cast<SmallObjectPoolHeader*>(bin) - 1;
        header->allocated = true;

        return bin;
    }

    uint8_t* aligned =
        util::alignUp(_current + offset + HeaderSize, alignment) - offset - HeaderSize;
    setupHeader(aligned, size);

    _current = aligned + size + HeaderSize;
    return aligned + HeaderSize;
}

void SmallObjectPool::free(void* data) {
    assert(("Freeing memory not associated to pool", _memory <= data && data <= _current));
    auto header = reinterpret_cast<SmallObjectPoolHeader*>(data) - 1;
    assert(("Freeing unallocated memory", header->allocated));
    assert(("Freeing invalid memory", header->size <= MaxSmallObjectSize));

#if !defined(NDEBUG) && !defined(_NDEBUG)
    memset(data, 0xDE, header->size);
#endif

    header->allocated = false;

    const size_t binIndex = header->size >> BinSizeBitShift;
    FreeList* bin = _free[binIndex];
    if (bin) {
        bin->next = reinterpret_cast<FreeList*>(data);
    } else {
        _free[binIndex] = reinterpret_cast<FreeList*>(data);
        _free[binIndex]->next = nullptr;
    }
}
