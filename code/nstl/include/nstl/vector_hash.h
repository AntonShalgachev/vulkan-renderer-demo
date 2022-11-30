#pragma once

#include "nstl/hash.h"
#include "nstl/vector.h"

template<typename T>
struct nstl::hash<nstl::vector<T>>
{
    size_t operator()(nstl::vector<T> const& value) const;
};

template<typename T>
size_t nstl::hash<nstl::vector<T>>::operator()(nstl::vector<T> const& value) const
{
    size_t hash = 0;
    for (auto const& v : value)
        hash_combine(hash, v);
    return hash;
}
