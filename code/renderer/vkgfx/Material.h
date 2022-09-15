#pragma once

#include "Handles.h"

namespace vkgfx
{
    struct Material
    {
        BufferHandle uniformBuffer;
        TextureHandle albedo;
        TextureHandle normalMap;
    };
}
