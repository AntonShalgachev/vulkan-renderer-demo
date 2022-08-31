#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <span>
#include "UniqueHandle.h"

namespace vko
{
    class Device;

    class ShaderModule
    {
    public:
        enum class Type
        {
            Vertex,
            Geometry,
            Fragment,
        };

    public:
        explicit ShaderModule(Device const& device, std::span<unsigned char const> bytes, Type type, std::string entryPoint);
        ~ShaderModule();

        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(ShaderModule&&) = default;

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        Device const& m_device;
        Type m_type;
        std::string m_entryPoint;

        UniqueHandle<VkShaderModule> m_handle;
    };
}
