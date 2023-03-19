#pragma once

#include "yyjsoncpp/serializer.h"
#include "yyjsoncpp/value_ref.h"

namespace yyjsoncpp
{
#define YYJSONCPP_DECLARE_SERIALIZER(T)                                        \
    template<> struct serializer<T>                                            \
    {                                                                          \
        static optional<T> from_json(value_ref value);                         \
        static mutable_value_ref to_json(mutable_doc& doc, T const& value);    \
    };

    YYJSONCPP_DECLARE_SERIALIZER(char const*);
    YYJSONCPP_DECLARE_SERIALIZER(nstl::string_view);

    template<size_t N>
    struct serializer<char[N]>
    {
        using T = char[N];
        static mutable_value_ref to_json(mutable_doc& doc, T const& value)
        {
            return serializer<char const*>::to_json(doc, value);
        }
    };

    YYJSONCPP_DECLARE_SERIALIZER(char);
    YYJSONCPP_DECLARE_SERIALIZER(signed char);
    YYJSONCPP_DECLARE_SERIALIZER(unsigned char);
    YYJSONCPP_DECLARE_SERIALIZER(short);
    YYJSONCPP_DECLARE_SERIALIZER(unsigned short);
    YYJSONCPP_DECLARE_SERIALIZER(int);
    YYJSONCPP_DECLARE_SERIALIZER(unsigned int);
    YYJSONCPP_DECLARE_SERIALIZER(long);
    YYJSONCPP_DECLARE_SERIALIZER(unsigned long);
    YYJSONCPP_DECLARE_SERIALIZER(long long);
    YYJSONCPP_DECLARE_SERIALIZER(unsigned long long);
    YYJSONCPP_DECLARE_SERIALIZER(float);
    YYJSONCPP_DECLARE_SERIALIZER(double);
    YYJSONCPP_DECLARE_SERIALIZER(long double);

    YYJSONCPP_DECLARE_SERIALIZER(bool);

#undef YYJSONCPP_DECLARE_SERIALIZER
}
