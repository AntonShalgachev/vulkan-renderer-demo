#pragma once

#include "editor/assets/Uuid.h"

#include "coil/Coil.h"

namespace coil
{
    template<>
    struct TypeSerializer<editor::assets::Uuid>
    {
        static Expected<editor::assets::Uuid, coil::String> fromString(Value const& input);
        static coil::String toString(editor::assets::Uuid const& value);
    };

    COIL_CREATE_TYPE_NAME_DECLARATION(editor::assets::Uuid);
}
