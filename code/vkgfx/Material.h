#pragma once

#include "vkgfx/Handles.h"

namespace vkgfx
{
    struct Material
    {
        BufferHandle uniformBuffer;
        TextureHandle albedo;
        TextureHandle normalMap;
    };
}
