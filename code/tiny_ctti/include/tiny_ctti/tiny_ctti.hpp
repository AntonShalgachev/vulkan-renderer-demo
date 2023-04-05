#pragma once

#if !defined(TINY_CTTI_CUSTOM_STRING_VIEW)
#include <string_view>
#define TINY_CTTI_CUSTOM_STRING_VIEW std::string_view
#endif

#if !defined(TINY_CTTI_CUSTOM_SPAN)
#include <span>
#define TINY_CTTI_CUSTOM_SPAN std::span
#endif

#if !defined(TINY_CTTI_CUSTOM_OPTIONAL)
#include <optional>
#define TINY_CTTI_CUSTOM_OPTIONAL std::optional
#endif

#if !defined(TINY_CTTI_CUSTOM_TUPLE)
#include <tuple>
#define TINY_CTTI_CUSTOM_TUPLE std::tuple
#endif

#if !defined(TINY_CTTI_CUSTOM_INDEX_SEQUENCE)
#include <utility>
#define TINY_CTTI_CUSTOM_INDEX_SEQUENCE std::index_sequence
#endif

#if !defined(TINY_CTTI_CUSTOM_MAKE_INDEX_SEQUENCE)
#include <utility>
#define TINY_CTTI_CUSTOM_MAKE_INDEX_SEQUENCE std::make_index_sequence
#endif

// TODO: support empty enums/structs

// Common
namespace tiny_ctti
{
    using string_view = TINY_CTTI_CUSTOM_STRING_VIEW;
    template<typename T> using span = TINY_CTTI_CUSTOM_SPAN<T>;
    template<typename T> using optional = TINY_CTTI_CUSTOM_OPTIONAL<T>;
    template<typename... Ts> using tuple = TINY_CTTI_CUSTOM_TUPLE<Ts...>;
    template<size_t... Is> using index_sequence = TINY_CTTI_CUSTOM_INDEX_SEQUENCE<Is...>;
    template<size_t Size> using make_index_sequence = TINY_CTTI_CUSTOM_MAKE_INDEX_SEQUENCE<Size>;

    template<typename T, size_t N>
    struct simple_array
    {
        T data[N];
        static constexpr size_t size = N;
    };

    struct dummy {};
    simple_array()->simple_array<dummy, 0>;
    template<typename T, typename... Ts>
    simple_array(T, Ts...) -> simple_array<T, sizeof...(Ts) + 1>;

    template<typename T> struct type_tag {};
    template<size_t I> struct index_tag {};

    namespace default_impl
    {
        template<typename T>
        constexpr bool tiny_ctti_is_described(type_tag<T>)
        {
            return false;
        }
    }

    template<typename T>
    constexpr bool is_described()
    {
        using namespace default_impl;
        return tiny_ctti_is_described(type_tag<T>{});
    }

    template<typename T> constexpr bool is_described_v = is_described<T>();
    template<typename T> constexpr string_view type_name_v = tiny_ctti_get_type_name<T>();

    template<typename T>
    constexpr string_view type_name()
    {
        static_assert(is_described_v<T>, "Type T is not described");
        return tiny_ctti_get_type_name(type_tag<T>{});
    }
}

// Enum
namespace tiny_ctti
{
    template<typename E>
    struct enum_entry
    {
        E value;
        string_view name;
    };

    namespace default_impl
    {
        template<typename E>
        constexpr bool tiny_ctti_is_enum(type_tag<E>)
        {
            return false;
        }
    }

    template<typename E>
    constexpr bool is_enum()
    {
        using namespace default_impl;
        return tiny_ctti_is_enum(type_tag<E>{});
    }

    template<typename E> constexpr bool is_enum_v = is_enum<E>();
    template<typename E> constexpr auto enum_entries_v = tiny_ctti_get_enum_entries(type_tag<E>{});
    template<typename E> constexpr size_t enum_size_v = enum_entries_v<E>.size;

    namespace detail
    {
        template<typename E>
        constexpr simple_array<E, enum_size_v<E>> create_enum_values()
        {
            simple_array<E, enum_size_v<E>> result;

            for (size_t i = 0; i < enum_entries_v<E>.size; i++)
                result.data[i] = enum_entries_v<E>.data[i].value;

            return result;
        };

        template<typename E>
        constexpr simple_array<string_view, enum_size_v<E>> create_enum_names()
        {
            simple_array<string_view, enum_size_v<E>> result;

            for (size_t i = 0; i < enum_entries_v<E>.size; i++)
                result.data[i] = enum_entries_v<E>.data[i].name;

            return result;
        }
    }

    template<typename E> constexpr simple_array<E, enum_size_v<E>> enum_values_v = detail::create_enum_values<E>();
    template<typename E> constexpr simple_array<string_view, enum_size_v<E>> enum_names_v = detail::create_enum_names<E>();

    template<typename E>
    constexpr span<enum_entry<E> const> enum_entries()
    {
        static_assert(is_enum_v<E>, "Enum E is not described");
        return { enum_entries_v<E>.data, enum_entries_v<E>.size };
    }

    template<typename E>
    constexpr size_t enum_count() noexcept
    {
        static_assert(is_enum_v<E>, "Enum E is not described");
        return enum_size_v<E>;
    }

    template<typename E>
    constexpr span<string_view const> enum_names()
    {
        static_assert(is_enum_v<E>, "Enum E is not described");
        return { enum_names_v<E>.data, enum_names_v<E>.size };
    }

    template<typename E>
    constexpr span<E const> enum_values()
    {
        static_assert(is_enum_v<E>, "Enum E is not described");
        return { enum_values_v<E>.data, enum_values_v<E>.size };
    }

    struct DefaultPredicate
    {
        bool operator()(string_view lhs, string_view rhs) const { return lhs == rhs; }
    };

    template<typename E, typename Pred = DefaultPredicate>
    constexpr optional<E> enum_cast(string_view name, Pred const& pred = {})
    {
        static_assert(is_enum_v<E>, "Enum E is not described");
        auto const& entries = enum_entries_v<E>;
        for (size_t i = 0; i < entries.size; i++)
            if (pred(entries.data[i].name, name))
                return entries.data[i].value;

        return {};
    }

    template<typename E>
    constexpr string_view enum_name(E value)
    {
        static_assert(is_enum_v<E>, "Enum E is not described");
        auto const& entries = enum_entries_v<E>;
        for (size_t i = 0; i < entries.size; i++)
            if (entries.data[i].value == value)
                return entries.data[i].name;

        return {};
    }
}

// Structs
namespace tiny_ctti
{
    template<typename O, typename T>
    struct struct_entry
    {
        T O::* field;
        string_view name;
    };

    template<typename O, typename T>
    struct_entry(T O::*, string_view) -> struct_entry<O, T>;

    namespace default_impl
    {
        template<typename T>
        constexpr bool tiny_ctti_is_struct(type_tag<T>)
        {
            return false;
        }
    }

    template<typename T>
    constexpr bool is_struct()
    {
        using namespace default_impl;
        return tiny_ctti_is_struct(type_tag<T>{});
    }

    template<typename T> constexpr bool is_struct_v = is_struct<T>();
    template<typename T> constexpr size_t struct_size_v = tiny_ctti_get_struct_size(type_tag<T>{});
    template<typename T, size_t I> constexpr auto struct_field_v = tiny_ctti_get_struct_field(type_tag<T>{}, index_tag<I>{});

    namespace detail
    {
        template<typename T, size_t... Is>
        constexpr auto create_struct_entries(index_sequence<Is...>)
        {
            return tuple<decltype(struct_field_v<T, Is>)...>{ struct_field_v<T, Is>... };
        }
    }

    template<typename T> constexpr auto struct_entries_v = detail::create_struct_entries<T>(make_index_sequence<struct_size_v<T>>{});

    template<typename T>
    constexpr size_t struct_size() noexcept
    {
        static_assert(is_struct_v<T>, "Struct T is not described");
        return struct_size_v<T>;
    }

    template<typename T>
    constexpr auto const& struct_entries() noexcept
    {
        static_assert(is_struct_v<T>, "Struct T is not described");
        return struct_entries_v<T>;
    }
}

//////////////////////////////////////////////////////////////////////////

#define TINY_CTTI_DESCRIBE_ENUM(E, ...)                                                                                                                                            \
constexpr bool tiny_ctti_is_described(tiny_ctti::type_tag<E>) { return true; }                                                                                                     \
constexpr bool tiny_ctti_is_enum(tiny_ctti::type_tag<E>) { return true; }                                                                                                          \
constexpr tiny_ctti::string_view tiny_ctti_get_type_name(tiny_ctti::type_tag<E>) { return { #E, sizeof(#E) - 1 }; }                                                                \
constexpr auto tiny_ctti_get_enum_entries(tiny_ctti::type_tag<E>) { return tiny_ctti::simple_array{ TCTTI_FOR_EACH(TCTTI_CREATE_ENUM_ENTRY, E, __VA_ARGS__) }; }

#define TINY_CTTI_DESCRIBE_STRUCT(T, ...)                                                                                                                                          \
constexpr bool tiny_ctti_is_described(tiny_ctti::type_tag<T>) { return true; }                                                                                                     \
constexpr bool tiny_ctti_is_struct(tiny_ctti::type_tag<T>) { return true; }                                                                                                        \
constexpr tiny_ctti::string_view tiny_ctti_get_type_name(tiny_ctti::type_tag<T>) { return { #T, sizeof(#T) - 1 }; }                                                                \
constexpr size_t tiny_ctti_get_struct_size(tiny_ctti::type_tag<T>) { return TCTTI_ARGS_COUNT(__VA_ARGS__); }                                                                       \
TCTTI_FOR_EACH(TCTTI_CREATE_STRUCT_FIELD, T, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////

#define TCTTI_CREATE_ENUM_ENTRY(T, I, X) tiny_ctti::enum_entry<T>{T::X, tiny_ctti::string_view{ #X, sizeof(#X) - 1 }},
#define TCTTI_CREATE_STRUCT_FIELD(T, I, X) constexpr auto tiny_ctti_get_struct_field(tiny_ctti::type_tag<T>, tiny_ctti::index_tag<I>) { return tiny_ctti::struct_entry{ &T::X, tiny_ctti::string_view{ #X, sizeof(#X) - 1 } }; }

//////////////////////////////////////////////////////////////////////////

#define TCTTI_EXPAND(X) X

#define TCTTI_GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112, _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125, N, ...) N
#define TCTTI_ARGS_COUNT(...) TCTTI_EXPAND(TCTTI_GET_NTH_ARG(__VA_ARGS__, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

#define TCTTI_CONCAT_IMPL(X, Y) X ## Y
#define TCTTI_CONCAT(X, Y) TCTTI_CONCAT_IMPL(X, Y)
#define TCTTI_GET_CALL(...) TCTTI_CONCAT(TCTTI_CALL_, TCTTI_ARGS_COUNT(__VA_ARGS__))

#define TCTTI_CALL_0(F, T, N, ...)
#define TCTTI_CALL_1(F, T, N, X)      F(T, (N-1), X)
#define TCTTI_CALL_2(F, T, N, X, ...) F(T, (N-2), X)        TCTTI_CALL_1(F, T, N,  __VA_ARGS__)
#define TCTTI_CALL_3(F, T, N, X, ...) F(T, (N-3), X) TCTTI_EXPAND(TCTTI_CALL_2(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_4(F, T, N, X, ...) F(T, (N-4), X) TCTTI_EXPAND(TCTTI_CALL_3(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_5(F, T, N, X, ...) F(T, (N-5), X) TCTTI_EXPAND(TCTTI_CALL_4(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_6(F, T, N, X, ...) F(T, (N-6), X) TCTTI_EXPAND(TCTTI_CALL_5(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_7(F, T, N, X, ...) F(T, (N-7), X) TCTTI_EXPAND(TCTTI_CALL_6(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_8(F, T, N, X, ...) F(T, (N-8), X) TCTTI_EXPAND(TCTTI_CALL_7(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_9(F, T, N, X, ...) F(T, (N-9), X) TCTTI_EXPAND(TCTTI_CALL_8(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_10(F, T, N, X, ...) F(T, (N-10), X) TCTTI_EXPAND(TCTTI_CALL_9(F, T, N, __VA_ARGS__))
#define TCTTI_CALL_11(F, T, N, X, ...) F(T, (N-11), X) TCTTI_EXPAND(TCTTI_CALL_10(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_12(F, T, N, X, ...) F(T, (N-12), X) TCTTI_EXPAND(TCTTI_CALL_11(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_13(F, T, N, X, ...) F(T, (N-13), X) TCTTI_EXPAND(TCTTI_CALL_12(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_14(F, T, N, X, ...) F(T, (N-14), X) TCTTI_EXPAND(TCTTI_CALL_13(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_15(F, T, N, X, ...) F(T, (N-15), X) TCTTI_EXPAND(TCTTI_CALL_14(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_16(F, T, N, X, ...) F(T, (N-16), X) TCTTI_EXPAND(TCTTI_CALL_15(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_17(F, T, N, X, ...) F(T, (N-17), X) TCTTI_EXPAND(TCTTI_CALL_16(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_18(F, T, N, X, ...) F(T, (N-18), X) TCTTI_EXPAND(TCTTI_CALL_17(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_19(F, T, N, X, ...) F(T, (N-19), X) TCTTI_EXPAND(TCTTI_CALL_18(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_20(F, T, N, X, ...) F(T, (N-20), X) TCTTI_EXPAND(TCTTI_CALL_19(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_21(F, T, N, X, ...) F(T, (N-21), X) TCTTI_EXPAND(TCTTI_CALL_20(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_22(F, T, N, X, ...) F(T, (N-22), X) TCTTI_EXPAND(TCTTI_CALL_21(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_23(F, T, N, X, ...) F(T, (N-23), X) TCTTI_EXPAND(TCTTI_CALL_22(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_24(F, T, N, X, ...) F(T, (N-24), X) TCTTI_EXPAND(TCTTI_CALL_23(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_25(F, T, N, X, ...) F(T, (N-25), X) TCTTI_EXPAND(TCTTI_CALL_24(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_26(F, T, N, X, ...) F(T, (N-26), X) TCTTI_EXPAND(TCTTI_CALL_25(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_27(F, T, N, X, ...) F(T, (N-27), X) TCTTI_EXPAND(TCTTI_CALL_26(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_28(F, T, N, X, ...) F(T, (N-28), X) TCTTI_EXPAND(TCTTI_CALL_27(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_29(F, T, N, X, ...) F(T, (N-29), X) TCTTI_EXPAND(TCTTI_CALL_28(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_30(F, T, N, X, ...) F(T, (N-30), X) TCTTI_EXPAND(TCTTI_CALL_29(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_31(F, T, N, X, ...) F(T, (N-31), X) TCTTI_EXPAND(TCTTI_CALL_30(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_32(F, T, N, X, ...) F(T, (N-32), X) TCTTI_EXPAND(TCTTI_CALL_31(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_33(F, T, N, X, ...) F(T, (N-33), X) TCTTI_EXPAND(TCTTI_CALL_32(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_34(F, T, N, X, ...) F(T, (N-34), X) TCTTI_EXPAND(TCTTI_CALL_33(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_35(F, T, N, X, ...) F(T, (N-35), X) TCTTI_EXPAND(TCTTI_CALL_34(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_36(F, T, N, X, ...) F(T, (N-36), X) TCTTI_EXPAND(TCTTI_CALL_35(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_37(F, T, N, X, ...) F(T, (N-37), X) TCTTI_EXPAND(TCTTI_CALL_36(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_38(F, T, N, X, ...) F(T, (N-38), X) TCTTI_EXPAND(TCTTI_CALL_37(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_39(F, T, N, X, ...) F(T, (N-39), X) TCTTI_EXPAND(TCTTI_CALL_38(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_40(F, T, N, X, ...) F(T, (N-40), X) TCTTI_EXPAND(TCTTI_CALL_39(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_41(F, T, N, X, ...) F(T, (N-41), X) TCTTI_EXPAND(TCTTI_CALL_40(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_42(F, T, N, X, ...) F(T, (N-42), X) TCTTI_EXPAND(TCTTI_CALL_41(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_43(F, T, N, X, ...) F(T, (N-43), X) TCTTI_EXPAND(TCTTI_CALL_42(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_44(F, T, N, X, ...) F(T, (N-44), X) TCTTI_EXPAND(TCTTI_CALL_43(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_45(F, T, N, X, ...) F(T, (N-45), X) TCTTI_EXPAND(TCTTI_CALL_44(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_46(F, T, N, X, ...) F(T, (N-46), X) TCTTI_EXPAND(TCTTI_CALL_45(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_47(F, T, N, X, ...) F(T, (N-47), X) TCTTI_EXPAND(TCTTI_CALL_46(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_48(F, T, N, X, ...) F(T, (N-48), X) TCTTI_EXPAND(TCTTI_CALL_47(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_49(F, T, N, X, ...) F(T, (N-49), X) TCTTI_EXPAND(TCTTI_CALL_48(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_50(F, T, N, X, ...) F(T, (N-50), X) TCTTI_EXPAND(TCTTI_CALL_49(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_51(F, T, N, X, ...) F(T, (N-51), X) TCTTI_EXPAND(TCTTI_CALL_50(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_52(F, T, N, X, ...) F(T, (N-52), X) TCTTI_EXPAND(TCTTI_CALL_51(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_53(F, T, N, X, ...) F(T, (N-53), X) TCTTI_EXPAND(TCTTI_CALL_52(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_54(F, T, N, X, ...) F(T, (N-54), X) TCTTI_EXPAND(TCTTI_CALL_53(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_55(F, T, N, X, ...) F(T, (N-55), X) TCTTI_EXPAND(TCTTI_CALL_54(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_56(F, T, N, X, ...) F(T, (N-56), X) TCTTI_EXPAND(TCTTI_CALL_55(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_57(F, T, N, X, ...) F(T, (N-57), X) TCTTI_EXPAND(TCTTI_CALL_56(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_58(F, T, N, X, ...) F(T, (N-58), X) TCTTI_EXPAND(TCTTI_CALL_57(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_59(F, T, N, X, ...) F(T, (N-59), X) TCTTI_EXPAND(TCTTI_CALL_58(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_60(F, T, N, X, ...) F(T, (N-60), X) TCTTI_EXPAND(TCTTI_CALL_59(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_61(F, T, N, X, ...) F(T, (N-61), X) TCTTI_EXPAND(TCTTI_CALL_60(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_62(F, T, N, X, ...) F(T, (N-62), X) TCTTI_EXPAND(TCTTI_CALL_61(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_63(F, T, N, X, ...) F(T, (N-63), X) TCTTI_EXPAND(TCTTI_CALL_62(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_64(F, T, N, X, ...) F(T, (N-64), X) TCTTI_EXPAND(TCTTI_CALL_63(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_65(F, T, N, X, ...) F(T, (N-65), X) TCTTI_EXPAND(TCTTI_CALL_64(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_66(F, T, N, X, ...) F(T, (N-66), X) TCTTI_EXPAND(TCTTI_CALL_65(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_67(F, T, N, X, ...) F(T, (N-67), X) TCTTI_EXPAND(TCTTI_CALL_66(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_68(F, T, N, X, ...) F(T, (N-68), X) TCTTI_EXPAND(TCTTI_CALL_67(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_69(F, T, N, X, ...) F(T, (N-69), X) TCTTI_EXPAND(TCTTI_CALL_68(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_70(F, T, N, X, ...) F(T, (N-70), X) TCTTI_EXPAND(TCTTI_CALL_69(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_71(F, T, N, X, ...) F(T, (N-71), X) TCTTI_EXPAND(TCTTI_CALL_70(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_72(F, T, N, X, ...) F(T, (N-72), X) TCTTI_EXPAND(TCTTI_CALL_71(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_73(F, T, N, X, ...) F(T, (N-73), X) TCTTI_EXPAND(TCTTI_CALL_72(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_74(F, T, N, X, ...) F(T, (N-74), X) TCTTI_EXPAND(TCTTI_CALL_73(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_75(F, T, N, X, ...) F(T, (N-75), X) TCTTI_EXPAND(TCTTI_CALL_74(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_76(F, T, N, X, ...) F(T, (N-76), X) TCTTI_EXPAND(TCTTI_CALL_75(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_77(F, T, N, X, ...) F(T, (N-77), X) TCTTI_EXPAND(TCTTI_CALL_76(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_78(F, T, N, X, ...) F(T, (N-78), X) TCTTI_EXPAND(TCTTI_CALL_77(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_79(F, T, N, X, ...) F(T, (N-79), X) TCTTI_EXPAND(TCTTI_CALL_78(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_80(F, T, N, X, ...) F(T, (N-80), X) TCTTI_EXPAND(TCTTI_CALL_79(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_81(F, T, N, X, ...) F(T, (N-81), X) TCTTI_EXPAND(TCTTI_CALL_80(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_82(F, T, N, X, ...) F(T, (N-82), X) TCTTI_EXPAND(TCTTI_CALL_81(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_83(F, T, N, X, ...) F(T, (N-83), X) TCTTI_EXPAND(TCTTI_CALL_82(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_84(F, T, N, X, ...) F(T, (N-84), X) TCTTI_EXPAND(TCTTI_CALL_83(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_85(F, T, N, X, ...) F(T, (N-85), X) TCTTI_EXPAND(TCTTI_CALL_84(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_86(F, T, N, X, ...) F(T, (N-86), X) TCTTI_EXPAND(TCTTI_CALL_85(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_87(F, T, N, X, ...) F(T, (N-87), X) TCTTI_EXPAND(TCTTI_CALL_86(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_88(F, T, N, X, ...) F(T, (N-88), X) TCTTI_EXPAND(TCTTI_CALL_87(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_89(F, T, N, X, ...) F(T, (N-89), X) TCTTI_EXPAND(TCTTI_CALL_88(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_90(F, T, N, X, ...) F(T, (N-90), X) TCTTI_EXPAND(TCTTI_CALL_89(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_91(F, T, N, X, ...) F(T, (N-91), X) TCTTI_EXPAND(TCTTI_CALL_90(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_92(F, T, N, X, ...) F(T, (N-92), X) TCTTI_EXPAND(TCTTI_CALL_91(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_93(F, T, N, X, ...) F(T, (N-93), X) TCTTI_EXPAND(TCTTI_CALL_92(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_94(F, T, N, X, ...) F(T, (N-94), X) TCTTI_EXPAND(TCTTI_CALL_93(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_95(F, T, N, X, ...) F(T, (N-95), X) TCTTI_EXPAND(TCTTI_CALL_94(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_96(F, T, N, X, ...) F(T, (N-96), X) TCTTI_EXPAND(TCTTI_CALL_95(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_97(F, T, N, X, ...) F(T, (N-97), X) TCTTI_EXPAND(TCTTI_CALL_96(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_98(F, T, N, X, ...) F(T, (N-98), X) TCTTI_EXPAND(TCTTI_CALL_97(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_99(F, T, N, X, ...) F(T, (N-99), X) TCTTI_EXPAND(TCTTI_CALL_98(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_100(F, T, N, X, ...) F(T, (N-100), X) TCTTI_EXPAND(TCTTI_CALL_99(F, , NT, __VA_ARGS__))
#define TCTTI_CALL_101(F, T, N, X, ...) F(T, (N-101), X) TCTTI_EXPAND(TCTTI_CALL_100(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_102(F, T, N, X, ...) F(T, (N-102), X) TCTTI_EXPAND(TCTTI_CALL_101(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_103(F, T, N, X, ...) F(T, (N-103), X) TCTTI_EXPAND(TCTTI_CALL_102(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_104(F, T, N, X, ...) F(T, (N-104), X) TCTTI_EXPAND(TCTTI_CALL_103(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_105(F, T, N, X, ...) F(T, (N-105), X) TCTTI_EXPAND(TCTTI_CALL_104(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_106(F, T, N, X, ...) F(T, (N-106), X) TCTTI_EXPAND(TCTTI_CALL_105(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_107(F, T, N, X, ...) F(T, (N-107), X) TCTTI_EXPAND(TCTTI_CALL_106(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_108(F, T, N, X, ...) F(T, (N-108), X) TCTTI_EXPAND(TCTTI_CALL_107(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_109(F, T, N, X, ...) F(T, (N-109), X) TCTTI_EXPAND(TCTTI_CALL_108(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_110(F, T, N, X, ...) F(T, (N-110), X) TCTTI_EXPAND(TCTTI_CALL_109(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_111(F, T, N, X, ...) F(T, (N-111), X) TCTTI_EXPAND(TCTTI_CALL_110(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_112(F, T, N, X, ...) F(T, (N-112), X) TCTTI_EXPAND(TCTTI_CALL_111(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_113(F, T, N, X, ...) F(T, (N-113), X) TCTTI_EXPAND(TCTTI_CALL_112(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_114(F, T, N, X, ...) F(T, (N-114), X) TCTTI_EXPAND(TCTTI_CALL_113(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_115(F, T, N, X, ...) F(T, (N-115), X) TCTTI_EXPAND(TCTTI_CALL_114(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_116(F, T, N, X, ...) F(T, (N-116), X) TCTTI_EXPAND(TCTTI_CALL_115(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_117(F, T, N, X, ...) F(T, (N-117), X) TCTTI_EXPAND(TCTTI_CALL_116(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_118(F, T, N, X, ...) F(T, (N-118), X) TCTTI_EXPAND(TCTTI_CALL_117(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_119(F, T, N, X, ...) F(T, (N-119), X) TCTTI_EXPAND(TCTTI_CALL_118(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_120(F, T, N, X, ...) F(T, (N-120), X) TCTTI_EXPAND(TCTTI_CALL_119(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_121(F, T, N, X, ...) F(T, (N-121), X) TCTTI_EXPAND(TCTTI_CALL_120(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_122(F, T, N, X, ...) F(T, (N-122), X) TCTTI_EXPAND(TCTTI_CALL_121(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_123(F, T, N, X, ...) F(T, (N-123), X) TCTTI_EXPAND(TCTTI_CALL_122(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_124(F, T, N, X, ...) F(T, (N-124), X) TCTTI_EXPAND(TCTTI_CALL_123(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_125(F, T, N, X, ...) F(T, (N-125), X) TCTTI_EXPAND(TCTTI_CALL_124(F,, N T, __VA_ARGS__))
#define TCTTI_CALL_126(F, T, N, X, ...) F(T, (N-126), X) TCTTI_EXPAND(TCTTI_CALL_125(F,, N T, __VA_ARGS__))

#define TCTTI_FOR_EACH(F, T, ...) TCTTI_EXPAND(TCTTI_GET_CALL(__VA_ARGS__)(F, T, TCTTI_ARGS_COUNT(__VA_ARGS__), __VA_ARGS__))
