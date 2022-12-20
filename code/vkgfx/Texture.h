#pragma once

#include "vkgfx/Handles.h"

namespace vkgfx
{
    struct Texture
    {
        ImageHandle image;
        SamplerHandle sampler;
    };
}
