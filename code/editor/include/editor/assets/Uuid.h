#pragma once

#include <stdint.h>

#include "common/fmt.h"

#include "nstl/hash.h"
#include "yyjsoncpp/serializer.h"

namespace nstl
{
    class string;
}

// TODO move out of editor::assets
// TODO have dedicated uuid-like classes for assets

namespace editor::assets
{
    struct Uuid
    {
        uint8_t bytes[16] = {};

        bool operator==(Uuid const&) const = default;
        explicit operator bool() const { return *this != Uuid{}; }

        nstl::string toString() const;
    };

    bool tryParseUuid(nstl::string_view str, Uuid& destination);
    Uuid generateUuid();

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

template<>
struct picofmt::formatter<editor::assets::Uuid> : public picofmt::formatter<nstl::string_view>
{
    bool format(editor::assets::Uuid const& value, context& ctx) const
    {
        return picofmt::formatter<nstl::string_view>::format(value.toString(), ctx);
    }
};
