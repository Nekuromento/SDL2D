#ifndef ScopeStack_h__
#define ScopeStack_h__

#include <cstdint>
#include <type_traits>
#include <utility>

#include "Util/defines.hpp"
#include "Util/noncopyable.hpp"

template <typename Allocator>
class ScopeStack : public util::Noncopyable {
    struct Finalizer {
        void (*destroy)(void* const);
        Finalizer* next;
    };

    template <class T>
    static void callDistructor(void* const instance) {
        static_cast<T*>(instance)->~T();
    }

    typedef size_t RewindMarker;

    Allocator& _alloc;
    const RewindMarker _rewindPoint;
    Finalizer* _finalizerChain;

    Finalizer* allocateWithFinalizer(const size_t size, const size_t alignment) NOEXCEPT {
        void* const memory =
            allocate(size + sizeof(Finalizer), alignment, sizeof(Finalizer));
        return static_cast<Finalizer*>(memory);
    }

    void* objectFromFinalizer(Finalizer* const finalizer) NOEXCEPT {
        return finalizer + 1;
    }

public:
    explicit ScopeStack(Allocator& alloc) NOEXCEPT :
        _alloc(alloc),
        _rewindPoint(alloc.rewindMarker()),
        _finalizerChain(nullptr)
    {}

    ~ScopeStack() {
        while (_finalizerChain) {
            _finalizerChain->destroy(objectFromFinalizer(_finalizerChain));
            _finalizerChain = _finalizerChain->next;
        }
        _alloc.rewind(_rewindPoint);
    }

    void* allocate(const size_t size, const size_t alignment, const size_t offset) NOEXCEPT {
        return _alloc.allocate(size, alignment, offset);
    }

    template <class C, typename... Args>
    C* create(Args&& ...args) {
        Finalizer* const finalizer =
            allocateWithFinalizer(sizeof(C), std::alignment_of<C>::value);

        C* const object =
            new (objectFromFinalizer(finalizer)) C(std::forward<Args>(args)...);

        finalizer->destroy = &callDistructor<C>;
        finalizer->next = _finalizerChain;
        _finalizerChain = finalizer;

        return object;
    }

    template <typename T>
    T* createPOD() NOEXCEPT {
        return static_cast<T*>(allocate(sizeof(T), std::alignment_of<T>::value, 0));
    }

    template <typename T>
    T* createPODArray(const size_t count) NOEXCEPT {
        return static_cast<T*>(allocate(sizeof(T) * count, std::alignment_of<T>::value, 0));
    }
};

#endif // ScopeStack_h__