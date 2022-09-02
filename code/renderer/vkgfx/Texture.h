#pragma once

#include "Handles.h"

namespace vkgfx
{
    struct Texture
    {
        ImageHandle image;
        SamplerHandle sampler;
    };
}
