#include "ShaderModule.h"

#include "Device.h"

namespace
{
    VkShaderStageFlagBits getStageFlags(vko::ShaderModule::Type type)
    {
        switch (type)
        {
        case vko::ShaderModule::Type::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case vko::ShaderModule::Type::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case vko::ShaderModule::Type::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        throw std::domain_error("getStageFlags: type has unsupported value");
    }
}

vko::ShaderModule::ShaderModule(Device const& device, std::span<unsigned char const> bytes, Type type, std::string entryPoint)
    : m_device(device)
    , m_type(type)
    , m_entryPoint(std::move(entryPoint))
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.data()); // TODO check if okay

    if (vkCreateShaderModule(m_device.getHandle(), &createInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");
}

vko::ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(m_device.getHandle(), m_handle, nullptr);
    m_handle = nullptr;
}

VkPipelineShaderStageCreateInfo vko::ShaderModule::createStageCreateInfo() const
{
    VkPipelineShaderStageCreateInfo stageCreateInfo{};
    stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCreateInfo.stage = getStageFlags(m_type);
    stageCreateInfo.module = m_handle;
    stageCreateInfo.pName = m_entryPoint.c_str();

    return stageCreateInfo;
}
