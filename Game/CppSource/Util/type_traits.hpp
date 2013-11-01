#ifndef type_traits_h__
#define type_traits_h__

#include <type_traits>

namespace util {
template <typename T>
struct False {
    static const bool value = false;
};

template <typename T>
class HasCallOperator {
    template <typename ReturnType, class C, typename... Args>
    static std::true_type test(ReturnType (C::*)(Args...) const);

    template <typename ReturnType, class C, typename... Args>
    static std::true_type test(ReturnType (C::*)(Args...));

    template <class C>
    static decltype(test(&C::operator())) test(decltype(&C::operator()), void*);

    template <class C>
    static std::false_type test(...);

public:
    static const bool value = std::is_same<std::true_type, decltype(test<T>(0, 0))>::value;
};

template <typename T>
struct FunctionType;

template <typename F>
typename FunctionType<F>::type* functionTypeHelper(F);

template <typename T>
struct FunctionType {
    static_assert(HasCallOperator<T>::value, "Only callable arguments are supported");

    typedef decltype(functionTypeHelper(&T::operator())) OperatorTypePtr;

    typedef typename std::remove_pointer<OperatorTypePtr>::type type;
};

template <typename ReturnType, typename... Args>
struct FunctionType<ReturnType (*)(Args...)> {
    typedef ReturnType type(Args...);
};

template <typename ReturnType, class C, typename... Args>
struct FunctionType<ReturnType (C::*)(Args...)> {
    typedef ReturnType type(Args...);
};

template <typename ReturnType, class C, typename... Args>
struct FunctionType<ReturnType (C::*)(Args...) const> {
    typedef ReturnType type(Args...);
};

template<typename T>
struct MemberFunctionClass {
    static_assert(False<T>::value, "Only member function arguments are supported");
};

template<class C, typename ReturnType, typename... Args>
struct MemberFunctionClass<ReturnType (C::*)(Args...)> {
    typedef C type;
};
}

#endif // type_traits_h__