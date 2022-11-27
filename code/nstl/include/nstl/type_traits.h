#pragma once

namespace nstl
{
    template<typename T>
    struct remove_cv
    {
        using Type = T;
    };
    template<typename T>
    struct remove_cv<T const>
    {
        using Type = T;
    };
    template<typename T>
    struct remove_cv<T volatile>
    {
        using Type = T;
    };
    template<typename T>
    struct remove_cv<T const volatile>
    {
        using Type = T;
    };
    template<typename T>
    using remove_cv_t = typename remove_cv<T>::Type;

    template<typename T>
    struct remove_reference
    {
        using Type = T;
    };
    template<typename T>
    struct remove_reference<T&>
    {
        using Type = T;
    };
    template<typename T>
    struct remove_reference<T&&>
    {
        using Type = T;
    };
    template<typename T>
    using remove_reference_t = typename remove_reference<T>::Type;

    template<typename T>
    struct remove_pointer
    {
        using Type = T;
    };
    template<typename T>
    struct remove_pointer<T*>
    {
        using Type = T;
    };
    template<typename T>
    struct remove_pointer<T* const>
    {
        using Type = T;
    };
    template<typename T>
    struct remove_pointer<T* volatile>
    {
        using Type = T;
    };
    template<typename T>
    struct remove_pointer<T* const volatile>
    {
        using Type = T;
    };
    template<typename T>
    using remove_pointer_t = typename remove_pointer<T>::Type;

//     template<typename T>
//     using DecayT = remove_cv_t<remove_reference_t<T>>;

    template<bool B, class T = void>
    struct enable_if
    {
    };
    template<class T>
    struct enable_if<true, T>
    {
        using Type = T;
    };
    template<bool B, class T = void>
    using enable_if_t = typename enable_if<B, T>::Type;

#if defined(__clang__)
    template<typename T1, typename T2>
    inline constexpr bool is_same_v = __is_same(T1, T2);
#else
    template<typename T1, typename T2>
    inline constexpr bool is_same_v = false;
    template<typename T>
    inline constexpr bool is_same_v<T, T> = true;
#endif

    template<typename T>
    inline constexpr bool is_void_v = is_same_v<remove_cv_t<T>, void>;

    template<typename T>
    using void_t = void;

    template<typename>
    constexpr bool is_const_v = false;
    template<typename T>
    constexpr bool is_const_v<const T> = true;

    template<class T>
    constexpr bool is_member_pointer_v = false;
    template<class T, class U>
    constexpr bool is_member_pointer_v<T U::*> = true;
}
