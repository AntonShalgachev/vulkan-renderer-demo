#pragma once

#include "nstl/sequence.h"
#include "nstl/utility.h"

namespace nstl
{
    template<size_t I, typename T>
    struct tuple_leaf
    {
        T value;
    };

    template<typename IndexSequence, typename... Ts>
    struct tuple_impl;

    template<size_t... Is, typename... Ts>
    struct tuple_impl<index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>...
    {
        static constexpr size_t size = sizeof...(Ts);

        template<size_t I, typename T>
        static tuple_leaf<I, T> leaf_type_getter(tuple_leaf<I, T> const&);

        template<size_t I>
        using leaf_type_at_index = decltype(leaf_type_getter<I>(declval<tuple_impl<index_sequence<Is...>, Ts...>>()));

        template<size_t I>
        constexpr auto const& get() const
        {
            return static_cast<leaf_type_at_index<I> const&>(*this).value;
        }
    };

    template<typename... Ts>
    struct tuple : tuple_impl<make_index_sequence<sizeof...(Ts)>, Ts...>
    {

    };

    template<typename... Args>
    tuple(Args...) -> tuple<Args...>;

    template<size_t I, typename... Ts>
    constexpr auto get(tuple<Ts...> const& t)
    {
        return t.template get<I>();
    }

    template<typename F, size_t... Is, typename... Ts>
    constexpr void apply(F const& f, tuple_impl<index_sequence<Is...>, Ts...> const& t)
    {
        f(t.template get<Is>()...);
    }
}
