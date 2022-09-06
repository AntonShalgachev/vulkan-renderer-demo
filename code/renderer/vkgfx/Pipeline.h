#pragma once

#include "wrapper/Pipeline.h"

namespace vkgfx
{
    struct Pipeline
    {
        Pipeline(vko::Pipeline pipeline);

        vko::Pipeline pipeline;
    };
}
