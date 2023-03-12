#pragma once

#include "yyjsoncpp/config.h"

namespace yyjsoncpp
{
    class value_ref;

    class mutable_doc;
    class mutable_value_ref;

    template<typename T, typename = void>
    struct serializer
    {
        static_assert(sizeof(T) < 0, "yyjsoncpp::serializer is not implemented for type T");

        static optional<T> from_json(value_ref value);
        static mutable_value_ref to_json(mutable_doc& doc, T const& obj);
    };
}
