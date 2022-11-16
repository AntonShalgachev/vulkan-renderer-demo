#pragma once

#include "UniqueHandle.h"

#include "nstl/span.h"

#include <vulkan/vulkan.h>

#include <string>

namespace vko
{
    class Device;

    enum class ShaderModuleType
    {
        Vertex,
        Geometry,
        Fragment,
    };

    class ShaderModule
    {
    public:
        explicit ShaderModule(Device const& device, nstl::span<unsigned char const> bytes, ShaderModuleType type, std::string entryPoint);
        ~ShaderModule();

        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(ShaderModule&&) = default;

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        VkDevice m_device;
        ShaderModuleType m_type;
        std::string m_entryPoint;

        UniqueHandle<VkShaderModule> m_handle;
    };
}
