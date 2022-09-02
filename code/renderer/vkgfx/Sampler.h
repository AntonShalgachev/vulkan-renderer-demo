#pragma once

#include "wrapper/Sampler.h"

namespace vkgfx
{
    struct Sampler
    {
        Sampler(vko::Sampler sampler);
        vko::Sampler sampler;
    };
}
