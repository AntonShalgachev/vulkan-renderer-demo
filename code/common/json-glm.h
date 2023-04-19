#pragma once

#include "common/glm.h"

#include "yyjsoncpp/serializer.h"

namespace yyjsoncpp
{
    template<>
    struct serializer<glm::vec3>
    {
        static optional<glm::vec3> from_json(value_ref obj);
        static mutable_value_ref to_json(mutable_doc& doc, glm::vec3 const& value);
    };

    template<>
    struct serializer<glm::vec4>
    {
        static optional<glm::vec4> from_json(value_ref obj);
        static mutable_value_ref to_json(mutable_doc& doc, glm::vec4 const& value);
    };

    template<>
    struct serializer<glm::quat>
    {
        static optional<glm::quat> from_json(value_ref obj);
        static mutable_value_ref to_json(mutable_doc& doc, glm::quat const& value);
    };

}
