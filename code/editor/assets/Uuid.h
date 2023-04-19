#pragma once

#include <stdint.h>

#include "nstl/hash.h"

#include "yyjsoncpp/serializer.h"

namespace nstl
{
    class string;
}

// TODO move out of editor::assets

namespace editor::assets
{
    struct Uuid
    {
        Uuid() = default;
        Uuid(nstl::string_view str);

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

template<>
struct yyjsoncpp::serializer<editor::assets::Uuid>
{
    static optional<editor::assets::Uuid> from_json(value_ref obj);
    static mutable_value_ref to_json(mutable_doc& doc, editor::assets::Uuid const& value);
};
