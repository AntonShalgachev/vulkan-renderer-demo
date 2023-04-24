#pragma once

#include "tglm/fwd.h"

#include "yyjsoncpp/serializer.h"

namespace yyjsoncpp
{
    template<>
    struct serializer<tglm::vec3>
    {
        static optional<tglm::vec3> from_json(value_ref obj);
        static mutable_value_ref to_json(mutable_doc& doc, tglm::vec3 const& value);
    };

    template<>
    struct serializer<tglm::vec4>
    {
        static optional<tglm::vec4> from_json(value_ref obj);
        static mutable_value_ref to_json(mutable_doc& doc, tglm::vec4 const& value);
    };

    template<>
    struct serializer<tglm::quat>
    {
        static optional<tglm::quat> from_json(value_ref obj);
        static mutable_value_ref to_json(mutable_doc& doc, tglm::quat const& value);
    };
}
