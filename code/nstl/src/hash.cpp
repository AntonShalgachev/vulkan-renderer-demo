#include "nstl/hash.h"

size_t nstl::computeStringHash(char const* bytes, size_t size)
{
    // djb2 from http://www.cse.yorku.ca/~oz/hash.html

    size_t hash = 5381;

    for (size_t i = 0; i < size; i++)
        hash = ((hash << 5) + hash) + bytes[i]; /* hash * 33 + c */

    return hash;
}

#define DEFINE_POS_HASH(T) \
    size_t nstl::hash<T>::operator()(T const& value) \
    { \
        return computeStringHash(reinterpret_cast<char const*>(&value), sizeof(value)); \
    }

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
