#ifndef SmallObjectPool_h__
#define SmallObjectPool_h__

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include "Util/noncopyable.hpp"

struct FreeList;

class SmallObjectPool : public util::Noncopyable {
    FreeList** _free;
    uint8_t* _memory;
    uint8_t* _current;
    size_t _size;

    static const size_t MaxSmallObjectSize = 128;
    static_assert(MaxSmallObjectSize <= 0xFF, "MaxSmallObjectSize is too big");

public:
    static const size_t BinSize = 8;
    static_assert((BinSize & (BinSize - 1)) == 0, "BinSize must be power of two");

private:
    static const size_t BinSizeBitShift = 3;
    static_assert(BinSize == (1 << BinSizeBitShift), "BinSize and BinSizeBitShift are out of sync");

    static const size_t BinCount = MaxSmallObjectSize / BinSize;
    static const size_t FreeListStorageSize = BinCount * sizeof(FreeList*);

public:
    struct DefaultInstance {
        static SmallObjectPool* defaultInstance;

        static const size_t DefaultPoolSize = 2 * 1024 * 1024;

        template <typename Allocator>
        DefaultInstance(Allocator& alloc) {
            assert(("Trying to initialize default instance twice", !defaultInstance));
            void* const memory =
                alloc.allocate(sizeof(SmallObjectPool), std::alignment_of<SmallObjectPool>::value, 0);
            defaultInstance = new (memory) SmallObjectPool(alloc, DefaultPoolSize);
        }
        ~DefaultInstance();
    };

    static SmallObjectPool& getDefault();

    template <typename Allocator>
    SmallObjectPool(Allocator& alloc, const size_t size) :
        _free {static_cast<FreeList**>(alloc.allocate(size + FreeListStorageSize, 1, FreeListStorageSize))},
        _memory {reinterpret_cast<uint8_t*>(_free) + FreeListStorageSize},
        _current {_memory},
        _size {size}
    {
        std::fill_n(_free, BinCount, nullptr);
    }
    ~SmallObjectPool();

    void* allocate(const size_t size, const size_t alignment, const size_t offset);
    void free(void* data);
};

#endif // SmallObjectPool_h__