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

#if defined(__GNUC__) || _MSC_VER >= 1800 // Detect VS 2012
template <typename ReturnType, typename... Args>
class Delegate<ReturnType (Args...)> {
    typedef ReturnType (* const Invoker)(void* const &, Args...);
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
    struct VtableBase {
        static REALLY_INLINE void destroy(void*& instance) NOEXCEPT {}

        static REALLY_INLINE void copy(void* const instance, void*& newInstance) NOEXCEPT {
            newInstance = instance;
        }
    };

    template <ReturnType (* Function)(Args...)>
    struct FunctionVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Args ...args) {
            return (Function)(args...);
        }

        static size_t value[VtableSize];
    };

    template <class C, ReturnType (C::* Method)(Args...)>
    struct MethodVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Args ...args) {
            return (static_cast<C*>(instance)->*Method)(args...);
        }

        static size_t value[VtableSize];
    };

    template <class Functor>
    struct FunctorVtable {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Args ...args) {
            if (CanBeStoredInline<Functor>::value) {
                return reinterpret_cast<Functor*>(const_cast<void**>(&instance))->operator()(args...);
            }
            return static_cast<Functor*>(instance)->operator()(args...);
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

    Delegate(size_t* const vtable, void* const instance) NOEXCEPT :
        vtable(vtable),
        instance(instance)
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
    Delegate() NOEXCEPT :
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

    Delegate(Delegate&& other) NOEXCEPT :
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

    Delegate& operator =(Delegate&& other) NOEXCEPT {
        if (&other != this) {
            other.swap(*this);
        }
        return *this;
    }

    inline bool operator ==(const Delegate& other) const NOEXCEPT {
        return vtable == other.vtable && instance == other.instance;
    }

    inline bool operator !=(const Delegate& other) const NOEXCEPT {
        return vtable != other.vtable && instance != other.instance;
    }

    inline operator UnspecifiedBoolType() const NOEXCEPT {
        return vtable == nullptr ? nullptr : &Delegate::instance;
    }

    void swap(Delegate& other) NOEXCEPT {
        std::swap(vtable, other.vtable);
        std::swap(instance, other.instance);
    }

    template <ReturnType (* Function)(Args...)>
    static Delegate create() NOEXCEPT {
        return Delegate(FunctionVtable<Function>::value, nullptr);
    }

    template <class C, ReturnType (C::* Method)(Args...)>
    static Delegate create(C* const instance) NOEXCEPT {
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

    template <typename... Params>
    ReturnType invoke(Params&& ...args) const {
        static_assert(sizeof...(Args) == sizeof...(Params), "Incorrect argument number");
        assert(vtable != nullptr);
        return reinterpret_cast<Invoker>(vtable[Invoke])(instance, std::forward<Params>(args)...);
    }

    size_t hash() const {
        return reinterpret_cast<size_t>(instance) ^ reinterpret_cast<size_t>(vtable);
    }
};

template <typename ReturnType, typename... Args>
template <ReturnType (* Function)(Args...)>
size_t Delegate<ReturnType (Args...)>::FunctionVtable<Function>::value[Delegate<ReturnType (Args...)>::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctionVtable<Function>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctionVtable<Function>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Args...)>::FunctionVtable<Function>::copy)
};

template <typename ReturnType, typename... Args>
template <class C, ReturnType (C::* Method)(Args...)>
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
#elif _MSC_VER == 1700
// Visual Studio currently cannot expand variadic arguments in function args
// so we provide delicious copypasta for zero, one, two and three argument cases
// also it cannot handle initializer lists in templates... so sad

#pragma region DelegateBase
class DelegateBase {
protected:
    typedef void (* Deleter)(void*&);
    typedef void (* Copier)(void* const, void*&);

    enum VtableFuntion {
        Invoke,
        Delete,
        Copy,
        LastFunction
    };
    typedef void* DelegateBase::*UnspecifiedBoolType;

    size_t* vtable;
    void* instance;

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

    DelegateBase(size_t* const vtable, void* const instance) :
        vtable { vtable },
        instance { instance }
    {}

    template <typename T>
    struct CanBeStoredInline {
        static const bool value = sizeof(T) <= sizeof(void*) &&
            (std::alignment_of<void*>::value % std::alignment_of<T>::value == 0);
    };
    
    struct VtableBase {
        static REALLY_INLINE void destroy(void*&) {};

        static REALLY_INLINE void copy(void* const instance, void*& newInstance) NOEXCEPT {
            newInstance = instance;
        }
    };

    template <class Functor>
    struct FunctorVtableBase {
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
        }
    };

public:
    static const size_t VtableSize = LastFunction;

    DelegateBase() :
        vtable {nullptr},
        instance {nullptr}
    {}

    ~DelegateBase() {
        clear();
    }

    size_t hash() const {
        return reinterpret_cast<size_t>(instance) ^ reinterpret_cast<size_t>(vtable);
    }
};
#pragma endregion DelegateBase

#pragma region Delegate0
template <typename ReturnType>
class Delegate<ReturnType ()> : public DelegateBase {
    typedef ReturnType (* const Invoker)(void* const &);

    template <ReturnType (* const Function)()>
    struct FunctionVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const &) {
            return (Function)();
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class C, ReturnType (C::* const Method)()>
    struct MethodVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance) {
            return (static_cast<C*>(instance)->*Method)();
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class Functor>
    struct FunctorVtable : public FunctorVtableBase<Functor> {
        static REALLY_INLINE ReturnType invoke(void* const & instance) {
            if (CanBeStoredInline<Functor>::value) {
                return reinterpret_cast<Functor*>(const_cast<void**>(&instance))->operator()();
            }
            return static_cast<Functor*>(instance)->operator()();
        }

        static size_t value[DelegateBase::VtableSize];
    };

    Delegate(size_t* const vtable, void* const instance) :
        DelegateBase(vtable, instance)
    {}

public:
    Delegate() :
        DelegateBase()
    {}

    Delegate(const Delegate& other) {
        vtable = other.vtable;
        other.copy(instance);
    }

    Delegate(Delegate&& other) :
        DelegateBase(other.vtable, other.instance)
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

    Delegate& operator =(Delegate&& other) {
        if (&other != this) {
            other.swap(*this);
        }
        return *this;
    }

    inline bool operator ==(const Delegate& other) const {
        return vtable == other.vtable && instance == other.instance;
    }

    inline bool operator !=(const Delegate& other) const {
        return vtable != other.vtable && instance != other.instance;
    }

    inline operator UnspecifiedBoolType() const {
        return vtable == nullptr ? nullptr : &Delegate::instance;
    }

    void swap(Delegate& other) {
        std::swap(vtable, other.vtable);
        std::swap(instance, other.instance);
    }

    template <ReturnType (* const Function)()>
    static Delegate create() {
        return Delegate(FunctionVtable<Function>::value, nullptr);
    }

    template <class C, ReturnType (C::* const Method)()>
    static Delegate create(C* const instance) {
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

    ReturnType invoke() const {
        assert(vtable != nullptr);
        return reinterpret_cast<Invoker>(vtable[Invoke])(instance);
    }
};

template <typename ReturnType>
template <ReturnType (* const Function)()>
size_t Delegate<ReturnType ()>::FunctionVtable<Function>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::FunctionVtable<Function>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::FunctionVtable<Function>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::FunctionVtable<Function>::copy)
};

template <typename ReturnType>
template <class C, ReturnType (C::* const Method)()>
size_t Delegate<ReturnType ()>::MethodVtable<C, Method>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::MethodVtable<C, Method>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::MethodVtable<C, Method>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::MethodVtable<C, Method>::copy)
};

template <typename ReturnType>
template <class Functor>
size_t Delegate<ReturnType ()>::FunctorVtable<Functor>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::FunctorVtable<Functor>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::FunctorVtable<Functor>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType ()>::FunctorVtable<Functor>::copy)
};
#pragma endregion Delegate0

#pragma region Delegate1
template <typename ReturnType, typename Arg>
class Delegate<ReturnType (Arg)> : public DelegateBase {
    typedef ReturnType (* const Invoker)(void* const &, Arg);

    template <ReturnType (* const Function)(Arg)>
    struct FunctionVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg) {
            return (Function)(arg);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class C, ReturnType (C::* const Method)(Arg)>
    struct MethodVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg) {
            return (static_cast<C*>(instance)->*Method)(arg);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class Functor>
    struct FunctorVtable : public FunctorVtableBase<Functor> {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg) {
            if (CanBeStoredInline<Functor>::value) {
                return reinterpret_cast<Functor*>(const_cast<void**>(&instance))->operator()(arg);
            }
            return static_cast<Functor*>(instance)->operator()(arg);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    Delegate(size_t* const vtable, void* const instance) :
        DelegateBase(vtable, instance)
    {}

public:
    Delegate() :
        DelegateBase()
    {}

    Delegate(const Delegate& other) {
        vtable = other.vtable;
        other.copy(instance);
    }

    Delegate(Delegate&& other) :
        DelegateBase(other.vtable, other.instance)
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

    Delegate& operator =(Delegate&& other) {
        if (&other != this) {
            other.swap(*this);
        }
        return *this;
    }

    inline bool operator ==(const Delegate& other) const {
        return vtable == other.vtable && instance == other.instance;
    }

    inline bool operator !=(const Delegate& other) const {
        return vtable != other.vtable && instance != other.instance;
    }

    inline operator UnspecifiedBoolType() const {
        return vtable == nullptr ? nullptr : &Delegate::instance;
    }

    void swap(Delegate& other) {
        std::swap(vtable, other.vtable);
        std::swap(instance, other.instance);
    }

    template <ReturnType (* const Function)(Arg)>
    static Delegate create() {
        return Delegate(FunctionVtable<Function>::value, nullptr);
    }

    template <class C, ReturnType (C::* const Method)(Arg)>
    static Delegate create(C* const instance) {
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

    template <typename Param>
    ReturnType invoke(Param&& arg) const {
        assert(vtable != nullptr);
        return reinterpret_cast<Invoker>(vtable[Invoke])(instance, std::forward<Param>(arg));
    }
};

template <typename ReturnType, typename Arg>
template <ReturnType (* const Function)(Arg)>
size_t Delegate<ReturnType (Arg)>::FunctionVtable<Function>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::FunctionVtable<Function>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::FunctionVtable<Function>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::FunctionVtable<Function>::copy)
};

template <typename ReturnType, typename Arg>
template <class C, ReturnType (C::* const Method)(Arg)>
size_t Delegate<ReturnType (Arg)>::MethodVtable<C, Method>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::MethodVtable<C, Method>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::MethodVtable<C, Method>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::MethodVtable<C, Method>::copy)
};

template <typename ReturnType, typename Arg>
template <class Functor>
size_t Delegate<ReturnType (Arg)>::FunctorVtable<Functor>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::FunctorVtable<Functor>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::FunctorVtable<Functor>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg)>::FunctorVtable<Functor>::copy)
};
#pragma endregion Delegate1

#pragma region Delegate2
template <typename ReturnType, typename Arg, typename Arg2>
class Delegate<ReturnType (Arg, Arg2)> : public DelegateBase {
    typedef ReturnType (* const Invoker)(void* const &, Arg, Arg2);

    template <ReturnType (* const Function)(Arg, Arg2)>
    struct FunctionVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg, Arg2 arg2) {
            return (Function)(arg, arg2);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class C, ReturnType (C::* const Method)(Arg, Arg2)>
    struct MethodVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg, Arg2 arg2) {
            return (static_cast<C*>(instance)->*Method)(arg, arg2);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class Functor>
    struct FunctorVtable : public FunctorVtableBase<Functor> {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg, Arg2 arg2) {
            if (CanBeStoredInline<Functor>::value) {
                return reinterpret_cast<Functor*>(const_cast<void**>(&instance))->operator()(arg, arg2);
            }
            return static_cast<Functor*>(instance)->operator()(arg, arg2);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    Delegate(size_t* const vtable, void* const instance) :
        DelegateBase(vtable, instance)
    {}

public:
    Delegate() :
        DelegateBase()
    {}

    Delegate(const Delegate& other) {
        vtable = other.vtable;
        other.copy(instance);
    }

    Delegate(Delegate&& other) :
        DelegateBase(other.vtable, other.instance)
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

    Delegate& operator =(Delegate&& other) {
        if (&other != this) {
            other.swap(*this);
        }
        return *this;
    }

    void swap(Delegate& other) {
        std::swap(vtable, other.vtable);
        std::swap(instance, other.instance);
    }

    inline bool operator ==(const Delegate& other) const {
        return vtable == other.vtable && instance == other.instance;
    }

    inline bool operator !=(const Delegate& other) const {
        return vtable != other.vtable && instance != other.instance;
    }

    inline operator UnspecifiedBoolType() const {
        return vtable == nullptr ? nullptr : &Delegate::instance;
    }

    template <ReturnType (* const Function)(Arg, Arg2)>
    static Delegate create() {
        return Delegate(FunctionVtable<Function>::value, nullptr);
    }

    template <class C, ReturnType (C::* const Method)(Arg, Arg2)>
    static Delegate create(C* const instance) {
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

    template <typename Param, typename Param2>
    ReturnType invoke(Param&& arg, Param2&& arg2) const {
        assert(vtable != nullptr);
        return reinterpret_cast<Invoker>(vtable[Invoke])(instance, std::forward<Param>(arg), std::forward<Param2>(arg2));
    }
};

template <typename ReturnType, typename Arg, typename Arg2>
template <ReturnType (* const Function)(Arg, Arg2)>
size_t Delegate<ReturnType (Arg, Arg2)>::FunctionVtable<Function>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::FunctionVtable<Function>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::FunctionVtable<Function>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::FunctionVtable<Function>::copy)
};

template <typename ReturnType, typename Arg, typename Arg2>
template <class C, ReturnType (C::* const Method)(Arg, Arg2)>
size_t Delegate<ReturnType (Arg, Arg2)>::MethodVtable<C, Method>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::MethodVtable<C, Method>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::MethodVtable<C, Method>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::MethodVtable<C, Method>::copy)
};

template <typename ReturnType, typename Arg, typename Arg2>
template <class Functor>
size_t Delegate<ReturnType (Arg, Arg2)>::FunctorVtable<Functor>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::FunctorVtable<Functor>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::FunctorVtable<Functor>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2)>::FunctorVtable<Functor>::copy)
};
#pragma endregion Delegate2

#pragma region Delegate3
template <typename ReturnType, typename Arg, typename Arg2, typename Arg3>
class Delegate<ReturnType (Arg, Arg2, Arg3)> : public DelegateBase {
    typedef ReturnType (* const Invoker)(void* const &, Arg, Arg2, Arg3);

    template <ReturnType (* const Function)(Arg, Arg2, Arg3)>
    struct FunctionVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg, Arg2 arg2, Arg3 arg3) {
            return (Function)(arg, arg2, arg3);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class C, ReturnType (C::* const Method)(Arg, Arg2, Arg3)>
    struct MethodVtable : public VtableBase {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg, Arg2 arg2, Arg3 arg3) {
            return (static_cast<C*>(instance)->*Method)(arg, arg2, arg3);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    template <class Functor>
    struct FunctorVtable : public FunctorVtableBase<Functor> {
        static REALLY_INLINE ReturnType invoke(void* const & instance, Arg arg, Arg2 arg2, Arg3 arg3) {
            if (CanBeStoredInline<Functor>::value) {
                return reinterpret_cast<Functor*>(const_cast<void**>(&instance))->operator()(arg, arg2, arg3);
            }
            return static_cast<Functor*>(instance)->operator()(arg, arg2, arg3);
        }

        static size_t value[DelegateBase::VtableSize];
    };

    Delegate(size_t* const vtable, void* const instance) :
        DelegateBase(vtable, instance)
    {}

public:
    Delegate() :
        DelegateBase()
    {}

    Delegate(const Delegate& other) {
        vtable = other.vtable;
        other.copy(instance);
    }

    Delegate(Delegate&& other) :
        DelegateBase(other.vtable, other.instance)
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

    Delegate& operator =(Delegate&& other) {
        if (&other != this) {
            other.swap(*this);
        }
        return *this;
    }

    void swap(Delegate& other) {
        std::swap(vtable, other.vtable);
        std::swap(instance, other.instance);
    }

    inline bool operator ==(const Delegate& other) const {
        return vtable == other.vtable && instance == other.instance;
    }

    inline bool operator !=(const Delegate& other) const {
        return vtable != other.vtable && instance != other.instance;
    }

    inline operator UnspecifiedBoolType() const {
        return vtable == nullptr ? nullptr : &Delegate::instance;
    }

    template <ReturnType (* const Function)(Arg, Arg2, Arg3)>
    static Delegate create() {
        return Delegate(FunctionVtable<Function>::value, nullptr);
    }

    template <class C, ReturnType (C::* const Method)(Arg, Arg2, Arg3)>
    static Delegate create(C* const instance) {
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

    template <typename Param, typename Param2, typename Param3>
    ReturnType invoke(Param&& arg, Param2&& arg2, Param3&& arg3) const {
        assert(vtable != nullptr);
        return reinterpret_cast<Invoker>(vtable[Invoke])(instance, std::forward<Param>(arg), std::forward<Param2>(arg2), std::forward<Param3>(arg3));
    }
};

template <typename ReturnType, typename Arg, typename Arg2, typename Arg3>
template <ReturnType (* const Function)(Arg, Arg2, Arg3)>
size_t Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctionVtable<Function>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctionVtable<Function>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctionVtable<Function>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctionVtable<Function>::copy)
};

template <typename ReturnType, typename Arg, typename Arg2, typename Arg3>
template <class C, ReturnType (C::* const Method)(Arg, Arg2, Arg3)>
size_t Delegate<ReturnType (Arg, Arg2, Arg3)>::MethodVtable<C, Method>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::MethodVtable<C, Method>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::MethodVtable<C, Method>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::MethodVtable<C, Method>::copy)
};

template <typename ReturnType, typename Arg, typename Arg2, typename Arg3>
template <class Functor>
size_t Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctorVtable<Functor>::value[DelegateBase::VtableSize] = {
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctorVtable<Functor>::invoke),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctorVtable<Functor>::destroy),
    reinterpret_cast<size_t>(&Delegate<ReturnType (Arg, Arg2, Arg3)>::FunctorVtable<Functor>::copy)
};
#pragma endregion Delegate3
#else
#error "Unsupported compiler"
#endif

template<class T>
struct you_forgot_to_define_method_in_header : public T
{};

#define THIS_TYPE \
    std::remove_reference<decltype(*this)>::type

#define THIS_FUNCTION_TYPE(FUNC) \
    util::FunctionType<decltype(&you_forgot_to_define_method_in_header<THIS_TYPE>::FUNC)>::type

#define THIS_CLASS_TYPE_FROM_FUNC(FUNC) \
    util::MemberFunctionClass<decltype(&you_forgot_to_define_method_in_header<THIS_TYPE>::FUNC)>::type

#define callback(FUNC) \
    Delegate<THIS_FUNCTION_TYPE(FUNC)>::create<THIS_CLASS_TYPE_FROM_FUNC(FUNC), &you_forgot_to_define_method_in_header<THIS_TYPE>::FUNC>(this)

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
