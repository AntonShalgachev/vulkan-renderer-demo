#pragma once

#include "renderer/vkgfx/Handles.h"

namespace vkgfx
{
    struct Texture
    {
        ImageHandle image;
        SamplerHandle sampler;
    };
}
