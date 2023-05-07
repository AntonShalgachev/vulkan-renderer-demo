#pragma once

#include "yyjsoncpp/config.h"

namespace yyjsoncpp
{
    class value_ref;

    class mutable_doc;
    class mutable_value_ref;

#if defined(__clang__)
    template<typename T1, typename T2> concept same_as = __is_same(T1, T2);
#else
    template<typename T1, typename T2> inline constexpr bool is_same_v = false;
    template<typename T> inline constexpr bool is_same_v<T, T> = true;
    template<typename T1, typename T2> concept same_as = is_same_v<T1, T2>;
#endif

    // The library expects the serializer to look like this:
    // struct serializer<T>
    // {
    //     static optional<T> from_json(value_ref value);
    //     static mutable_value_ref to_json(mutable_doc& doc, T const& obj);
    // };

    template<typename T>
    struct serializer;

    template<typename T>
    concept serializable = requires(value_ref value, mutable_doc& doc, T const& obj)
    {
        { serializer<T>::from_json(value) } -> same_as<optional<T>>;
        { serializer<T>::to_json(doc, obj) } -> same_as<mutable_value_ref>;
    };
}
