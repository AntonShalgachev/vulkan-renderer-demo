#pragma once

#include "renderer/vkgfx/Handles.h"

namespace vkgfx
{
    struct Material
    {
        BufferHandle uniformBuffer;
        TextureHandle albedo;
        TextureHandle normalMap;
    };
}
