#pragma once

#include "coil/Coil.h" // TODO forward-declare?
#include "glm-fwd.h"

namespace coil
{
    // TODO cover other glm types
    template<>
    struct TypeSerializer<glm::vec3>
    {
        static size_t const N = 3;
        using ElementType = float;

        static Expected<glm::vec3, coil::String> fromString(Value const& input);

        static coil::String toString(glm::vec3 const& value);
    };

    COIL_CREATE_TYPE_NAME_DECLARATION(glm::vec3);
}
