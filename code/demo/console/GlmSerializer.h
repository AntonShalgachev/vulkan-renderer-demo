#pragma once

#include "coil/Coil.h" // TODO forward-declare?

#include "tglm/fwd.h"

namespace coil
{
    // TODO cover other glm types
    template<>
    struct TypeSerializer<tglm::vec3>
    {
        static size_t const N = 3;
        using ElementType = float;

        static Expected<tglm::vec3, coil::String> fromString(Value const& input);

        static coil::String toString(tglm::vec3 const& value);
    };

    COIL_CREATE_TYPE_NAME_DECLARATION(tglm::vec3);
}
