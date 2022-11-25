#pragma once

#include "stddef.h"

namespace nstl
{
    template<typename T>
    struct hash
    {
        static_assert(sizeof(T) < 0, "No specialization of hash for type T");
        size_t operator()(T const& value);
    };

    size_t computeStringHash(char const* bytes, size_t size);

#define DECLARE_POD_HASH(T) \
    template<> \
    struct hash<T> \
    { \
        size_t operator()(T const& value); \
    };

    DECLARE_POD_HASH(bool);
    DECLARE_POD_HASH(char);
    DECLARE_POD_HASH(signed char);
    DECLARE_POD_HASH(unsigned char);
    DECLARE_POD_HASH(short);
    DECLARE_POD_HASH(unsigned short);
    DECLARE_POD_HASH(int);
    DECLARE_POD_HASH(unsigned int);
    DECLARE_POD_HASH(long);
    DECLARE_POD_HASH(unsigned long);
    DECLARE_POD_HASH(long long);
    DECLARE_POD_HASH(unsigned long long);
    DECLARE_POD_HASH(float);
    DECLARE_POD_HASH(double);
    DECLARE_POD_HASH(long double);
}
