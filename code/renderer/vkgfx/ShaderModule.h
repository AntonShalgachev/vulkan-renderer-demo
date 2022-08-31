#pragma once

#include "wrapper/ShaderModule.h"

namespace vkgfx
{
    struct ShaderModuleHandle
    {
        std::size_t index; // TODO improve
    };

    struct ShaderModule
    {
        ShaderModule(vko::ShaderModule shaderModule);

        vko::ShaderModule shaderModule;
    };
}
