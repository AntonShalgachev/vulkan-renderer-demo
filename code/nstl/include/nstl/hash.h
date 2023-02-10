#pragma once

#include "stddef.h"
#include "type_traits.h"

namespace nstl
{
    template<typename T, typename = void>
    struct hash
    {
        static_assert(sizeof(T) < 0, "No specialization of hash for type T");
        size_t operator()(T const& value);
    };

    size_t hash_string(char const* bytes, size_t size);
    void hash_combine(size_t& hash, size_t hash2);

    template<typename T>
    size_t hash_array(T* values, size_t size)
    {
        size_t hash = 0;
        for (size_t i = 0; i < size; i++)
            hash_combine(hash, values[i]);
        return hash;
    }

    template<typename T>
    void hash_combine(size_t& hash, T const& v)
    {
        nstl::hash<T> hasher;
        hash_combine(hash, hasher(v));
    }

    template<typename... Ts>
    size_t hash_values(Ts const&... values)
    {
        size_t hash = 0;
        (hash_combine(hash, values), ...);
        return hash;
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

template<>
struct nstl::hash<char const*>
{
    size_t operator()(char const* value) const;
};

template<typename U, size_t N>
struct nstl::hash<U[N]>
{
    using T = U[N];
    size_t operator()(T const& value) const
    {
        return hash_array(value, N);
    }
};

template<size_t N>
struct nstl::hash<char[N]>
{
    using T = char[N];
    size_t operator()(T const& value) const
    {
        return hash_string(value, N);
    }
};

template<typename E>
struct nstl::hash<E, nstl::enable_if_t<nstl::is_enum_v<E>>>
{
    size_t operator()(E const& value) const
    {
        using T = nstl::underlying_type_t<E>;
        return nstl::hash<T>{}(static_cast<T>(value));
    }
};
