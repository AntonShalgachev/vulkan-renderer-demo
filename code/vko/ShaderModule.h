#pragma once

#include "vko/UniqueHandle.h"

#include "nstl/span.h"
#include "nstl/string.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Device;

    enum class ShaderModuleType;

    class ShaderModule
    {
    public:
        explicit ShaderModule(Device const& device, nstl::span<unsigned char const> bytes, ShaderModuleType type, nstl::string entryPoint);
        ~ShaderModule();

        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(ShaderModule&&) = default;

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        VkDevice m_device;
        ShaderModuleType m_type;
        nstl::string m_entryPoint;

        UniqueHandle<VkShaderModule> m_handle;
    };
}
