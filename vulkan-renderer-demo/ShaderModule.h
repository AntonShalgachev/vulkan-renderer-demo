#pragma once

#include "framework.h"

namespace vkr
{
    class ShaderModule
    {
    public:
        enum class Type
        {
            Vertex,
            Fragment,
        };

    public:
        ShaderModule(std::string const& path);
        ~ShaderModule();

        VkPipelineShaderStageCreateInfo createStageCreateInfo(Type type, char const* entryPoint);

        VkShaderModule getHandle() const { return m_handle; }

    private:
        VkShaderModule m_handle;
        std::string m_entryPoint;
    };
}

