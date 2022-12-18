#pragma once

#include <assert.h>

#if defined(__clang__)
#define CHARMING_ENUM_IS_CLANG (1)
#else
#define CHARMING_ENUM_IS_CLANG (0)
#endif

#if defined(__GNUC__)
#define CHARMING_ENUM_IS_GCC (1)
#else
#define CHARMING_ENUM_IS_GCC (0)
#endif

#if defined(_MSC_VER)
#define CHARMING_ENUM_IS_MSVC (1)
#else
#define CHARMING_ENUM_IS_MSVC (0)
#endif

// Clang: 5.0.0+
// GCC: 9.1+
// MSVC: 19.20+

#if CHARMING_ENUM_IS_CLANG
    #define CHARMING_ENUM_FUNCTION_NAME __PRETTY_FUNCTION__
    #if __clang_major__ >= 8
        #define CHARMING_ENUM_HAS_MEMCPY_INTRINSICS 1
    #else
        #define CHARMING_ENUM_HAS_MEMCPY_INTRINSICS 0
    #endif
    #define CHARMING_ENUM_HAS_MEMCHR_INTRINSICS 1
    #define CHARMING_ENUM_HAS_MAKE_INTEGER_SEQ_INTRINSICS 1
    #define CHARMING_ENUM_HAS_INTEGER_PACK_INTRINSICS 0
#elif CHARMING_ENUM_IS_GCC
    #define CHARMING_ENUM_FUNCTION_NAME __PRETTY_FUNCTION__
    #define CHARMING_ENUM_HAS_MEMCPY_INTRINSICS 0
    #define CHARMING_ENUM_HAS_MEMCHR_INTRINSICS 0
    #if defined(__has_builtin)
        #if __has_builtin(__make_integer_seq)
            #define CHARMING_ENUM_HAS_MAKE_INTEGER_SEQ_INTRINSICS 1
        #else
            #define CHARMING_ENUM_HAS_MAKE_INTEGER_SEQ_INTRINSICS 0
        #endif
    #else
        #define CHARMING_ENUM_HAS_MAKE_INTEGER_SEQ_INTRINSICS 0
    #endif
    #define CHARMING_ENUM_HAS_INTEGER_PACK_INTRINSICS 1
#elif CHARMING_ENUM_IS_MSVC
    #define CHARMING_ENUM_FUNCTION_NAME __FUNCSIG__
    #define CHARMING_ENUM_HAS_MEMCPY_INTRINSICS 0
    #define CHARMING_ENUM_HAS_MEMCHR_INTRINSICS 1
    #define CHARMING_ENUM_HAS_MAKE_INTEGER_SEQ_INTRINSICS 1
    #define CHARMING_ENUM_HAS_INTEGER_PACK_INTRINSICS 0
#else
    #error Unsupported compiler
#endif

#if !defined(CHARMING_ENUM_CUSTOM_STRING_VIEW)
#include <string_view>
#define CHARMING_ENUM_CUSTOM_STRING_VIEW std::string_view
#endif

#if !defined(CHARMING_ENUM_CUSTOM_SPAN)
#include <span>
#define CHARMING_ENUM_CUSTOM_SPAN std::span<T>
#endif

#if !defined(CHARMING_ENUM_CUSTOM_OPTIONAL)
#include <optional>
#define CHARMING_ENUM_CUSTOM_OPTIONAL std::optional<T>
#endif

#if defined(CHARMING_ENUM_NO_CONSTEXPR)
#define CHARMING_ENUM_CONSTEXPR
#else
#define CHARMING_ENUM_CONSTEXPR constexpr
#endif

namespace charming_enum
{
    using string_view = CHARMING_ENUM_CUSTOM_STRING_VIEW;
    template<typename T> using span = CHARMING_ENUM_CUSTOM_SPAN;
    template<typename T> using optional = CHARMING_ENUM_CUSTOM_OPTIONAL;

    template<typename E>
    struct enum_entry_view
    {
        string_view name;
        E value;
    };

    namespace customize
    {
#if defined(CHARMING_ENUM_DEFAULT_RANGE_MIN) && defined(CHARMING_ENUM_DEFAULT_RANGE_MAX)
        static_assert(CHARMING_ENUM_DEFAULT_RANGE_MAX >= CHARMING_ENUM_DEFAULT_RANGE_MIN, "Invalid default range (CHARMING_ENUM_DEFAULT_RANGE_MIN and CHARMING_ENUM_DEFAULT_RANGE_MAX)");
        template<typename E>
        struct enum_range
        {
            static constexpr int min = CHARMING_ENUM_DEFAULT_RANGE_MIN;
            static constexpr int max = CHARMING_ENUM_DEFAULT_RANGE_MAX;
        };
#else
        template<typename E>
        struct enum_range
        {
            static_assert(sizeof(E) < 0, "customize::enum_range doesn't have a specialization of type E. Either explicitly provide a specialization for type E, or define both CHARMING_ENUM_DEFAULT_RANGE_MIN and CHARMING_ENUM_DEFAULT_RANGE_MAX before including this file");
        };
#endif
    }

    namespace detail
    {
        template<typename T, T... Is> struct integer_sequence {};
        template<size_t... Is> using index_sequence = integer_sequence<size_t, Is...>;
#if CHARMING_ENUM_HAS_MAKE_INTEGER_SEQ_INTRINSICS
        template<typename T, size_t S> using make_integer_sequence = __make_integer_seq<integer_sequence, T, S>;
#elif CHARMING_ENUM_HAS_INTEGER_PACK_INTRINSICS
        template<typename T, size_t S> using make_integer_sequence = integer_sequence<T, __integer_pack(S)...>;
#else
        #error Unsupported compiler
#endif
        template<size_t S> using make_index_sequence = make_integer_sequence<size_t, S>;

        struct simple_string_view
        {
            char const* data;
            size_t length;
        };

        constexpr size_t npos = static_cast<size_t>(-1);

        template<typename T>
        struct simple_span
        {
            T* data;
            size_t size;
        };

        template<typename T, size_t N>
        struct simple_array
        {
            T data[N];
            static constexpr size_t size = N;
        };

        //////////////////////////////////////////////////////////////////////////

        CHARMING_ENUM_CONSTEXPR char const* find_char(char const* haystack, int needle, size_t size)
        {
#if CHARMING_ENUM_HAS_MEMCHR_INTRINSICS
            return __builtin_char_memchr(haystack, needle, size);
#endif

            for (size_t i = 0; i < size; i++)
                if (haystack[i] == needle)
                    return haystack + i;
            return nullptr;
        }

        CHARMING_ENUM_CONSTEXPR bool is_identifier_char(char c)
        {
            return (
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                (c == '_')
                );
        }

        CHARMING_ENUM_CONSTEXPR bool is_first_identifier_char(char c)
        {
            return (
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c == '_')
                );
        }

        CHARMING_ENUM_CONSTEXPR simple_string_view get_enum_name(simple_string_view param)
        {
            // invalid: "(SomeEnumType)-42", "-42", "((anonymous namespace)::SomeEnumType)-42", "(ns::SomeEnumType)-42"
            // valid: "(anonymous namespace)::some_function()::SomeEnumType::Some_Value42"

            for (size_t i = param.length; i > 0; --i)
            {
                if (!is_identifier_char(param.data[i - 1]))
                {
                    param.data += i;
                    param.length -= i;
                    break;
                }
            }

            if (param.length > 0 && is_first_identifier_char(param.data[0]))
                return param;

            return {};
        }

        CHARMING_ENUM_CONSTEXPR size_t parse_names_list(simple_string_view list, simple_span<simple_string_view> names, simple_span<int> values, int value_offset)
        {
            size_t offset = 0;
            size_t values_written = 0;
            size_t count = 0;
            size_t value = value_offset;

            while (true)
            {
                // TODO replace list.data + offset with the variable
                char const* comma_char = find_char(list.data + offset, ',', list.length - offset);
                size_t comma_pos = comma_char ? (comma_char - list.data) : npos;

                size_t param_end = comma_pos;
                if (comma_pos == npos)
                    param_end = list.length;

                assert(param_end >= offset);
                simple_string_view name = get_enum_name({ list.data + offset, param_end - offset });

                if (name.length > 0)
                {
                    assert(count < names.size);
                    assert(count < values.size);

                    names.data[count] = name;
                    values.data[count] = value;
                    count++;
                }

                if (!comma_char)
                    break;

                value++;

                offset = comma_pos + 1;
                assert(offset < list.length);

                if (list.data[offset] == ' ')
                    offset++;

                assert(offset < list.length);
            }

            return count;
        }

        CHARMING_ENUM_CONSTEXPR size_t parse_pretty_function_brackets(simple_string_view name, simple_span<simple_string_view> names, simple_span<int> values, int value_offset, char opening_bracket, char closing_bracket) noexcept
        {
            // auto ns::some_function() [Es = <Value1, Value2, Value3>]

            char const* names_begin_char = find_char(name.data, opening_bracket, name.length);
            assert(names_begin_char);

            char const* names_end_char = find_char(names_begin_char, closing_bracket, name.length);
            assert(names_end_char);

            assert(names_begin_char < names_end_char);

            simple_string_view names_list = { names_begin_char + 1, static_cast<size_t>(names_end_char - names_begin_char - 1) };

            return parse_names_list(names_list, names, values, value_offset);
        }

        CHARMING_ENUM_CONSTEXPR simple_string_view parse_type_pretty_function_equals(simple_string_view name) noexcept
        {
            // auto ns::some_function() [T = SomeType]

            char const* equals_char = find_char(name.data, '=', name.length);
            assert(equals_char && equals_char >= name.data);

            size_t equals_pos = equals_char - name.data;

            assert(equals_pos + 2 <= name.length && name.data[equals_pos + 1] == ' ');
            size_t name_begin_pos = equals_pos + 2;

            assert(name.data[name.length - 1] == ']');

            return { name.data + name_begin_pos, name.length - name_begin_pos - 1 };
        }

        CHARMING_ENUM_CONSTEXPR size_t parse_pretty_function(simple_string_view name, simple_span<simple_string_view> names, simple_span<int> values, int value_offset) noexcept
        {
            // clang:
            // auto charming_enum::detail::pretty_function() [Es = <((anonymous namespace)::Enum1)-1, (anonymous namespace)::Enum1::Value1, (anonymous namespace)::Enum1::Value2, ((anonymous namespace)::Enum1)2>]

            // gcc:
            // constexpr auto charming_enum::detail::pretty_function() [with auto ...Es = {(<unnamed>::Enum1)-1, <unnamed>::Enum1::Value1, <unnamed>::Enum1::Value2, (<unnamed>::Enum1)2}]

            // msvc:
            // auto __cdecl charming_enum::detail::pretty_function<(enum `anonymous-namespace'::Enum1)0xffffffff,`anonymous-namespace'::Enum1::Value1,`anonymous-namespace'::Enum1::Value2,(enum `anonymous-namespace'::Enum1)0x2>(void) noexcept

#if CHARMING_ENUM_IS_CLANG
            return parse_pretty_function_brackets(name, names, values, value_offset, '<', '>');
#elif CHARMING_ENUM_IS_GCC
            return parse_pretty_function_brackets(name, names, values, value_offset, '{', '}');
#elif CHARMING_ENUM_IS_MSVC
            return parse_pretty_function_brackets(name, names, values, value_offset, '<', '>');
#endif
        }

        CHARMING_ENUM_CONSTEXPR simple_string_view parse_type_pretty_function(simple_string_view pretty_function_str) noexcept
        {
            // clang:
            // auto charming_enum::detail::pretty_function() [T = (anonymous namespace)::Enum1]

            // gcc:
            // constexpr auto charming_enum::detail::pretty_function() [with T = {anonymous}::Enum1]

            // msvc:
            // auto __cdecl charming_enum::detail::pretty_function<enum `anonymous-namespace'::Enum1>(void) noexcept

#if CHARMING_ENUM_IS_CLANG
            return parse_type_pretty_function_equals(pretty_function_str);
#elif CHARMING_ENUM_IS_GCC
            return parse_type_pretty_function_equals(pretty_function_str);
#elif CHARMING_ENUM_IS_MSVC
            simple_string_view name{};
            int value = 0;
            parse_pretty_function_brackets(pretty_function_str, { &name, 1 }, { &value, 1 }, 0, '<', '>');
            return name;
#endif
        }

        //////////////////////////////////////////////////////////////////////////

        template<typename E>
        struct enum_range_info
        {
            static_assert(customize::enum_range<E>::max >= customize::enum_range<E>::min, "Invalid customize::enum_range");

            static constexpr int min = customize::enum_range<E>::min;
            static constexpr size_t size = customize::enum_range<E>::max - customize::enum_range<E>::min + 1;
        };

        template<size_t RangeSize>
        struct parse_result
        {
            simple_array<simple_string_view, RangeSize> names;
            simple_array<int, RangeSize> values;
            size_t count;
        };

        // TODO optimize templates for several enums with the same range

        template<auto... Es>
        constexpr auto pretty_function() noexcept
        {
            return simple_string_view{ CHARMING_ENUM_FUNCTION_NAME, sizeof(CHARMING_ENUM_FUNCTION_NAME) - 1 };
        }

        template<typename T>
        constexpr auto pretty_function() noexcept
        {
            return simple_string_view{ CHARMING_ENUM_FUNCTION_NAME, sizeof(CHARMING_ENUM_FUNCTION_NAME) - 1 };
        }

        template<typename E>
        using parse_result_t = parse_result<enum_range_info<E>::size>;

        template<typename E, int... Is>
        constexpr parse_result_t<E> parse_pretty_function_for(integer_sequence<int, Is...>) noexcept
        {
            simple_string_view pretty_function_str = pretty_function<static_cast<E>(enum_range_info<E>::min + Is)...>();

            parse_result_t<E> result{};
            result.count = parse_pretty_function(pretty_function_str, { result.names.data, result.names.size }, { result.values.data, result.values.size }, enum_range_info<E>::min);
            return result;
        }
    }

    namespace detail
    {
#if !defined(CHARMING_ENUM_NO_CONSTEXPR)
        namespace compiletime
        {
            template<size_t N>
            struct static_string
            {
#if CHARMING_ENUM_HAS_MEMCPY_INTRINSICS
                constexpr static_string(simple_string_view sv)
                {
                    __builtin_memcpy(data, sv.data, N);
                }
#else
                constexpr static_string(simple_string_view sv) : static_string(make_index_sequence<N>{}, sv)
                {
                    
                }

                template<size_t... Is>
                constexpr static_string(index_sequence<Is...>, simple_string_view sv) : data{ sv.data[Is]... }
                {

                }
#endif

                constexpr operator string_view() const { return string_view{ data, N }; }

                char data[N];
            };

            template<typename T, typename U = T&&>
            U declval_impl();
            template<typename T>
            auto declval() noexcept -> decltype(declval_impl<T>());

            template<size_t Index, typename Type>
            struct simple_tuple_leaf
            {
                Type value;
            };

            template<typename IndexSequence, typename... Ts>
            struct simple_tuple_impl;

            template<size_t... Is, typename... Ts>
            struct simple_tuple_impl<index_sequence<Is...>, Ts...> : simple_tuple_leaf<Is, Ts>...
            {
                static constexpr size_t size = sizeof...(Ts);

                template<size_t I, typename T>
                static simple_tuple_leaf<I, T> leaf_type_getter(simple_tuple_leaf<I, T> const&);

                template<size_t I>
                using leaf_type_at_index = decltype(leaf_type_getter<I>(declval<simple_tuple_impl<index_sequence<Is...>, Ts...>>()));

                template<size_t I>
                constexpr auto const& get() const
                {
                    return static_cast<leaf_type_at_index<I> const&>(*this).value;
                }
            };

            template<typename E>
            constexpr parse_result_t<E> parse_result_v = parse_pretty_function_for<E>(make_integer_sequence<int, enum_range_info<E>::size>{});

            template<typename E>
            constexpr size_t enum_cardinality_v = parse_result_v<E>.count;

            template<typename E>
            constexpr simple_string_view enum_type_name_view_v = parse_type_pretty_function(pretty_function<E>());

            template<typename E>
            constexpr size_t enum_type_name_length = enum_type_name_view_v<E>.length;

            template<typename E>
            constexpr static_string<enum_type_name_length<E>> enum_type_name_v = enum_type_name_view_v<E>;

            //////////////////////////////////////////////////////////////////////////

            template<size_t N>
            struct enum_entry_storage
            {
                static_string<N> name;
                int value; // TODO other type?
            };

            template<typename E, typename IndexSequence>
            struct storage_type_helper;

            template<typename E, size_t... Is>
            struct storage_type_helper<E, index_sequence<Is...>>
            {
                using type = simple_tuple_impl<index_sequence<Is...>, enum_entry_storage<parse_result_v<E>.names.data[Is].length>...>;
            };

            template<typename E>
            using enum_name_storage_t = typename storage_type_helper<E, make_index_sequence<enum_cardinality_v<E>>>::type;

            template<typename E, size_t... Is>
            constexpr enum_name_storage_t<E> create_entry_storage(index_sequence<Is...>)
            {
                return { enum_entry_storage<parse_result_v<E>.names.data[Is].length>{parse_result_v<E>.names.data[Is], parse_result_v<E>.values.data[Is] }... }; // TODO make prettier
            }

            template<typename E>
            constexpr enum_name_storage_t<E> entry_storage_v = create_entry_storage<E>(make_index_sequence<enum_cardinality_v<E>>{});

            template<typename E, size_t... Is>
            constexpr simple_array<enum_entry_view<E>, enum_cardinality_v<E>> create_entry_views(index_sequence<Is...>)
            {
                return { enum_entry_view<E>{entry_storage_v<E>.template get<Is>().name, static_cast<E>(entry_storage_v<E>.template get<Is>().value) }... }; // TODO make prettier
            }

            template<typename E>
            constexpr simple_array<enum_entry_view<E>, enum_cardinality_v<E>> entry_views_v = create_entry_views<E>(make_index_sequence<enum_cardinality_v<E>>{});

            template<typename E, size_t... Is>
            constexpr simple_array<string_view, enum_cardinality_v<E>> create_names(index_sequence<Is...>)
            {
                return { entry_views_v<E>.data[Is].name... };
            }

            template<typename E>
            constexpr simple_array<string_view, enum_cardinality_v<E>> names_v = create_names<E>(make_index_sequence<enum_cardinality_v<E>>{});

            template<typename E, size_t... Is>
            constexpr simple_array<E, enum_cardinality_v<E>> create_values(index_sequence<Is...>)
            {
                return { entry_views_v<E>.data[Is].value... };
            }

            template<typename E>
            constexpr simple_array<E, enum_cardinality_v<E>> values_v = create_values<E>(make_index_sequence<enum_cardinality_v<E>>{});
        }
#endif

#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        namespace runtime
        {
            template<typename E>
            parse_result_t<E> const& get_parse_result()
            {
                static parse_result_t<E> parse_result = parse_pretty_function_for<E>(make_integer_sequence<int, enum_range_info<E>::size>{});
                return parse_result;
            }

            template<typename E>
            size_t const enum_cardinality_v = get_parse_result<E>().count;

            template<typename E>
            simple_array<enum_entry_view<E>, enum_range_info<E>::size> compute_entry_views()
            {
                auto const& parse_result = get_parse_result<E>();
                simple_array<enum_entry_view<E>, enum_range_info<E>::size> all_entries;
                for (size_t i = 0; i < parse_result.count; i++)
                {
                    simple_string_view raw_name = parse_result.names.data[i];
                    int raw_value = parse_result.values.data[i];
                    all_entries.data[i] = enum_entry_view<E>{ string_view{raw_name.data, raw_name.length}, static_cast<E>(raw_value) };
                }
                return all_entries;
            }

            template<typename E>
            simple_array<enum_entry_view<E>, enum_range_info<E>::size> const& get_entry_views()
            {
                static simple_array<enum_entry_view<E>, enum_range_info<E>::size> entries = compute_entry_views<E>();
                return entries;
            }

            template<typename E>
            simple_array<string_view, enum_range_info<E>::size> compute_names()
            {
                auto const& parse_result = get_parse_result<E>();
                simple_array<string_view, enum_range_info<E>::size> all_names;
                for (size_t i = 0; i < parse_result.count; i++)
                {
                    simple_string_view raw_name = parse_result.names.data[i];
                    all_names.data[i] = string_view{ raw_name.data, raw_name.length };
                }
                return all_names;
            }

            template<typename E>
            simple_array<E, enum_range_info<E>::size> compute_values()
            {
                auto const& parse_result = get_parse_result<E>();
                simple_array<E, enum_range_info<E>::size> all_values;
                for (size_t i = 0; i < parse_result.count; i++)
                    all_values.data[i] = static_cast<E>(parse_result.values.data[i]);
                return all_values;
            }
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////

    template<typename E>
    CHARMING_ENUM_CONSTEXPR string_view enum_type_name()
    {
#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        static detail::simple_string_view type_name_view = detail::parse_type_pretty_function(detail::pretty_function<E>());
        return { type_name_view.data, type_name_view.length };
#else
        return detail::compiletime::enum_type_name_v<E>;
#endif
    }

    template<typename E>
    CHARMING_ENUM_CONSTEXPR span<enum_entry_view<E> const> enum_entries()
    {
#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        return { detail::runtime::get_entry_views<E>().data, detail::runtime::enum_cardinality_v<E> };
#else
        return { detail::compiletime::entry_views_v<E>.data, detail::compiletime::entry_views_v<E>.size };
#endif
    }

    template<typename E>
    CHARMING_ENUM_CONSTEXPR size_t enum_count() noexcept
    {
#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        return detail::runtime::enum_cardinality_v<E>;
#else
        return detail::compiletime::enum_cardinality_v<E>;
#endif
    }

    template<typename E>
    CHARMING_ENUM_CONSTEXPR span<string_view const> enum_names()
    {
#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        static detail::simple_array<string_view, detail::enum_range_info<E>::size> const& names = detail::runtime::compute_names<E>();
        return { names.data, detail::runtime::enum_cardinality_v<E> };
#else
        return { detail::compiletime::names_v<E>.data, detail::compiletime::names_v<E>.size };
#endif
    }

    template<typename E>
    CHARMING_ENUM_CONSTEXPR span<E const> enum_values()
    {
#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        static detail::simple_array<E, detail::enum_range_info<E>::size> const& values = detail::runtime::compute_values<E>();
        return { values.data, detail::runtime::enum_cardinality_v<E> };
#else
        return { detail::compiletime::values_v<E>.data, detail::compiletime::values_v<E>.size };
#endif
    }

    struct DefaultPredicate
    {
        bool operator()(string_view lhs, string_view rhs) const { return lhs == rhs; }
    };

    template<typename E, typename Pred = DefaultPredicate>
    CHARMING_ENUM_CONSTEXPR optional<E> enum_cast(string_view name, Pred const& pred = {})
    {
#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        auto const& entries = detail::runtime::get_entry_views<E>();
        for (size_t i = 0; i < detail::runtime::enum_cardinality_v<E>; i++)
            if (pred(entries.data[i].name, name))
                return entries.data[i].value;
#else
        auto const& entries = detail::compiletime::entry_views_v<E>;
        for (size_t i = 0; i < entries.size; i++)
            if (pred(entries.data[i].name, name))
                return entries.data[i].value;
#endif

        return {};
    }

    template<typename E>
    CHARMING_ENUM_CONSTEXPR string_view enum_name(E value)
    {
#if defined(CHARMING_ENUM_NO_CONSTEXPR)
        auto const& entries = detail::runtime::get_entry_views<E>();
        for (size_t i = 0; i < detail::runtime::enum_cardinality_v<E>; i++)
            if (entries.data[i].value == value)
                return entries.data[i].name;
#else
        auto const& entries = detail::compiletime::entry_views_v<E>;
        for (size_t i = 0; i < entries.size; i++)
            if (entries.data[i].value == value)
                return entries.data[i].name;
#endif

        return {};
    }
}
