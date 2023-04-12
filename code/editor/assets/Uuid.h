#pragma once

#include <stdint.h>

#include "nstl/hash.h"

namespace nstl
{
    class string;
}

// TODO move out of editor::assets

namespace editor::assets
{
    struct Uuid
    {
        uint8_t bytes[16] = {};

        bool operator==(Uuid const&) const = default;
        operator bool() const { return *this != Uuid{}; }

        static Uuid generate();
        nstl::string toString() const;
    };

    static_assert(sizeof(Uuid) == 16);
}

template<>
struct nstl::hash<editor::assets::Uuid>
{
    size_t operator()(editor::assets::Uuid const& value) const;
};
