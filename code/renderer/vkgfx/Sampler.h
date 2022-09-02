#pragma once

#include "wrapper/Sampler.h"

namespace vkgfx
{
    struct SamplerHandle
    {
        std::size_t index = 0; // TODO improve
    };

    struct Sampler
    {
        Sampler(vko::Sampler sampler);
        vko::Sampler sampler;
    };
}
