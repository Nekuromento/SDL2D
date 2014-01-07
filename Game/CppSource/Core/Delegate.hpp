#ifndef Delegate_h__
#define Delegate_h__

#include <cassert>
#include <type_traits>
#include <utility>

#include "Core/Memory/SmallObjectPool.hpp"
#include "Util/defines.hpp"
#include "Util/type_traits.hpp"

template <typename T>
class Delegate {
    static_assert(util::False<T>::value, "Only callable arguments are supported");
};

template <typename ReturnType, typename... Args>
class Delegate<ReturnType (Args...)> {
    typedef ReturnType (* const Invoker)(void* const &, Args&&...);
    typedef void (* Deleter)(void*&);
    typedef void* (* Copier)(void* const, void*&);

    enum VtableFuntion {
        Invoke,
        Delete,
        Copy,
        LastFunction
    };

    template <typename T>
    struct CanBeStoredInline {
        static const bool value = sizeof(T) <= sizeof(void*) &&
            (std::alignment_of<void*>::value % std::alignment_of<T>::value == 0);
    };

    typedef void* Delegate::*UnspecifiedBoolType;

public:
    static const size_t VtableSize = LastFunction;

private:
    template <ReturnType (* const Function)(Args...)>
    struct FunctionVtable {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Args&& ...args) {
            return (Function)(std::forward<Args>(args)...);
        }

        static REALLY_INLINE void destroy(void*&) noexcept {}

        static REALLY_INLINE void copy(void* const instance, void*& newInstance) noexcept {
            newInstance = instance;
        }

        static size_t value[VtableSize];
    };

    template <class C, ReturnType (C::* const Method)(Args...)>
    struct MethodVtable {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Args&& ...args) {
            return (static_cast<C*>(instance)->*Method)(std::forward<Args>(args)...);
        }

        static REALLY_INLINE void destroy(void*&) noexcept {}

        static REALLY_INLINE void copy(void* const instance, void*& newInstance) noexcept {
            newInstance = instance;
        }

        static size_t value[VtableSize];
    };

    template <class Functor>
    struct FunctorVtable {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Args&& ...args) {
            if (CanBeStoredInline<Functor>::value) {
                return reinterpret_cast<Functor*>(const_cast<void**>(&instance))->operator()(std::forward<Args>(args)...);
            }
            return static_cast<Functor*>(instance)->operator()(std::forward<Args>(args)...);
        }

        static REALLY_INLINE void destroy(void*& instance) {
            if (CanBeStoredInline<Functor>::value) {
                reinterpret_cast<Functor*>(const_cast<void**>(&instance))->~Functor();
            } else {
                static_cast<Functor*>(instance)->~Functor();
                SmallObjectPool::getDefault().free(instance);
            }
        }

        static REALLY_INLINE void copy(void* const instance, void*& newInstance) {
            if (CanBeStoredInline<Functor>::value) {
                new (&newInstance) Functor(*reinterpret_cast<Functor*>(const_cast<void**>(&instance)));
            } else {
                void* memory =
                    SmallObjectPool::getDefault().allocate(sizeof(Functor), std::alignment_of<Functor>::value, 0);
                newInstance = new (memory) Functor(*static_cast<Functor*>(instance));
            }
        };

        static size_t value[VtableSize];
    };

    size_t* vtable;
    void* instance;

    Delegate(size_t* const vtable, void* const instance) noexcept :
        vtable {vtable},
        instance {instance}
    {}

    void clear() {
        if (vtable) {
            reinterpret_cast<Deleter>(vtable[Delete])(instance);
        }
    }

    void copy(void*& newInstance) const {
        if (vtable) {
            reinterpret_cast<Copier>(vtable[Copy])(instance, newInstance);
        } else {
            newInstance = nullptr;
        }
    }

public:
    Delegate() noexcept :
        vtable {nullptr},
        instance {nullptr}
    {}

    ~Delegate() {
        clear();
    }

    Delegate(const Delegate& other) :
        vtable {other.vtable},
        instance {nullptr}
    {
        other.copy(instance);
    }

    Delegate(Delegate&& other) noexcept :
        vtable {other.vtable},
        instance {other.instance}
    {
        other.vtable = nullptr;
        other.instance = nullptr;
    }

    Delegate& operator =(const Delegate& other) {
        if (&other != this) {
            Delegate(other).swap(*this);
        }
        return *this;
    }

    Delegate& operator =(Delegate&& other) noexcept {
        if (&other != this) {
            other.swap(*this);
        }
        return *this;
    }

    inline bool operator ==(const Delegate& other) const noexcept {
        return vtable == other.vtable && instance == other.instance;
    }

    inline bool operator !=(const Delegate& other) const noexcept {
        return vtable != other.vtable && instance != other.instance;
    }

    inline operator UnspecifiedBoolType() const noexcept {
        return vtable == nullptr ? nullptr : &Delegate::instance;
    }

    void swap(Delegate& other) noexcept {
        std::swap(vtable, other.vtable);
        std::swap(instance, other.instance);
    }

    template <ReturnType (* const Function)(Args...)>
    static Delegate create() noexcept {
        return Delegate(FunctionVtable<Function>::value, nullptr);
    }

    template <class C, ReturnType (C::* const Method)(Args...)>
    static Delegate create(C* const instance) noexcept {
        return Delegate(MethodVtable<C, Method>::value, instance);
    }

    template <class Functor>
    static Delegate create(Functor&& functor) {
        if (CanBeStoredInline<Functor>::value) {
            Delegate created(FunctorVtable<Functor>::value, nullptr);
            new (&created.instance) Functor(std::forward<Functor>(functor));
            return created;
        }
        void* memory =
            SmallObjectPool::getDefault().allocate(sizeof(Functor), std::alignment_of<Functor>::value, 0);
        return Delegate(FunctorVtable<Functor>::value,
                        new (memory) Functor(std::forward<Functor>(functor)));
    }

    ReturnType invoke(Args&& ...args) const {
        assert(vtable != nullptr);
        return reinterpret_cast<Invoker>(vtable[Invoke])(instance, std::forward<Args>(args)...);
    }
};

template <typename ReturnType, typename... Args>
template <ReturnType (* const Function)(Args...)>
size_t Delegate<ReturnType (Args...)>::FunctionVtable<Function>::value[Delegate<ReturnType (Args...)>::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctionVtable<Function>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctionVtable<Function>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctionVtable<Function>::copy)
};

template <typename ReturnType, typename... Args>
template <class C, ReturnType (C::* const Method)(Args...)>
size_t Delegate<ReturnType (Args...)>::MethodVtable<C, Method>::value[Delegate<ReturnType (Args...)>::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::MethodVtable<C, Method>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::MethodVtable<C, Method>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::MethodVtable<C, Method>::copy)
};

template <typename ReturnType, typename... Args>
template <class Functor>
size_t Delegate<ReturnType (Args...)>::FunctorVtable<Functor>::value[Delegate<ReturnType (Args...)>::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctorVtable<Functor>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctorVtable<Functor>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctorVtable<Functor>::copy)
};

#define THIS_TYPE std::remove_reference<decltype(*this)>::type
#define THIS_FUNCTION_TYPE(FUNC) util::FunctionType<decltype(&THIS_TYPE::FUNC)>::type
#define THIS_CLASS_TYPE_FROM_FUNC(FUNC) util::MemberFunctionClass<decltype(&THIS_TYPE::FUNC)>::type

#define callback(FUNC) \
    Delegate<THIS_FUNCTION_TYPE(FUNC)>::create<THIS_CLASS_TYPE_FROM_FUNC(FUNC), &THIS_TYPE::FUNC>(this)

#define static_callback(FUNC) \
    Delegate<util::FunctionType<decltype(&FUNC)>::type>::create<&FUNC>()

template <typename Functor>
Delegate<typename util::FunctionType<Functor>::type> make_delegate(Functor&& func) {
    static_assert(util::HasCallOperator<Functor>::value, "Only callable arguments are supported");

    return Delegate<typename util::FunctionType<Functor>::type>::create(std::forward<Functor>(func));
}

template <typename ReturnType, typename... Args>
Delegate<ReturnType (Args...)> make_delegate(ReturnType (*func)(Args...)) {
    return make_delegate([=](Args&& ...args) {
        return func(std::forward<Args>(args)...);
    });
}

template <class C, typename ReturnType, typename... Args>
Delegate<ReturnType (Args...)> make_delegate(C* instance, ReturnType (C::*method)(Args...)) {
    return make_delegate([=](Args&& ...args) {
        return (instance->*method)(std::forward<Args>(args)...);
    });
}

#endif // Delegate_h__
