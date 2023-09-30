#include "nstl/hash.h"

#include <string.h>

size_t nstl::hash_string(char const* bytes, size_t size)
{
    // djb2 from http://www.cse.yorku.ca/~oz/hash.html

    size_t hash = 5381;

    for (size_t i = 0; i < size; i++)
        hash = hash * 33 + static_cast<size_t>(bytes[i]);

    return hash;
}

void nstl::hash_combine(size_t& hash, size_t hash2)
{
    hash ^= hash2 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

#define DEFINE_POS_HASH(T) \
    size_t nstl::hash<T>::operator()(T const& value) \
    { \
        return hash_string(reinterpret_cast<char const*>(&value), sizeof(value)); \
    }

DEFINE_POS_HASH(void*);
DEFINE_POS_HASH(bool);
DEFINE_POS_HASH(char);
DEFINE_POS_HASH(signed char);
DEFINE_POS_HASH(unsigned char);
DEFINE_POS_HASH(short);
DEFINE_POS_HASH(unsigned short);
DEFINE_POS_HASH(int);
DEFINE_POS_HASH(unsigned int);
DEFINE_POS_HASH(long);
DEFINE_POS_HASH(unsigned long);
DEFINE_POS_HASH(long long);
DEFINE_POS_HASH(unsigned long long);
DEFINE_POS_HASH(float);
DEFINE_POS_HASH(double);
DEFINE_POS_HASH(long double);

#undef DEFINE_POS_HASH

namespace nstl
{
    size_t hash<char const*>::operator()(char const* value) const
    {
        return hash_string(value, strlen(value));
    }
}
