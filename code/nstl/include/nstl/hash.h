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

    size_t hash_string(char const* bytes, size_t size);
    void hash_combine(size_t& hash, size_t hash2);

    template<typename T>
    void hash_combine(size_t& hash, T const& v)
    {
        nstl::hash<T> hasher;
        hash_combine(hash, hasher(v));
    }

    template<typename... Ts>
    size_t hash_values(Ts const&... values)
    {
        size_t seed = 0;
        (hash_combine(seed, values), ...);
        return seed;
    }
}

#define DECLARE_POD_HASH(T) \
template<> \
struct nstl::hash<T> \
{ \
    size_t operator()(T const& value); \
};

DECLARE_POD_HASH(void*);
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

#undef DECLARE_POD_HASH
