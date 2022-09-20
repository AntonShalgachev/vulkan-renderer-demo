#include "ShaderModule.h"

#include "Device.h"

#include <cassert>

namespace
{
    VkShaderStageFlagBits getStageFlags(vko::ShaderModuleType type)
    {
        switch (type)
        {
        case vko::ShaderModuleType::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case vko::ShaderModuleType::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case vko::ShaderModuleType::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        throw std::domain_error("getStageFlags: type has unsupported value");
    }
}

vko::ShaderModule::ShaderModule(Device const& device, std::span<unsigned char const> bytes, ShaderModuleType type, std::string entryPoint)
    : m_device(device)
    , m_type(type)
    , m_entryPoint(std::move(entryPoint))
{
    assert(bytes.size() % 4 == 0);
    static_assert(sizeof(uint32_t) == 4 * sizeof(unsigned char));

    std::vector<uint32_t> code;
    code.resize(bytes.size() / 4);
    memcpy(code.data(), bytes.data(), bytes.size());

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = code.data();

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
