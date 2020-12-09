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
        ShaderModule(std::string const& path, Type type, std::string const& entryPoint);
        ~ShaderModule();

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        VkShaderModule m_handle = VK_NULL_HANDLE;

        Type m_type;
        std::string m_entryPoint;
    };
}

