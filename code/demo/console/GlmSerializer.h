#pragma once

#include "coil/Coil.h" // TODO forward-declare?

#include "tglm/fwd.h"

namespace coil
{
    template<>
    struct TypeSerializer<tglm::vec2>
    {
        static Expected<tglm::vec2, coil::String> fromString(Value const& input);
        static coil::String toString(tglm::vec2 const& value);
    };
    COIL_CREATE_TYPE_NAME_DECLARATION(tglm::vec2);

    template<>
    struct TypeSerializer<tglm::vec3>
    {
        static Expected<tglm::vec3, coil::String> fromString(Value const& input);
        static coil::String toString(tglm::vec3 const& value);
    };
    COIL_CREATE_TYPE_NAME_DECLARATION(tglm::vec3);

    template<>
    struct TypeSerializer<tglm::vec4>
    {
        static Expected<tglm::vec4, coil::String> fromString(Value const& input);
        static coil::String toString(tglm::vec4 const& value);
    };
    COIL_CREATE_TYPE_NAME_DECLARATION(tglm::vec4);
}
