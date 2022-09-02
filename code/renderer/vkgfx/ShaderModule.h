#pragma once

#include "wrapper/ShaderModule.h"

namespace vkgfx
{
    struct ShaderModule
    {
        ShaderModule(vko::ShaderModule shaderModule);

        vko::ShaderModule shaderModule;
    };
}
