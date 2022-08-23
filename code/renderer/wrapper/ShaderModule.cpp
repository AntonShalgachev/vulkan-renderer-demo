#include "ShaderModule.h"

#include <fstream>

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

    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        std::streamsize fileSize = file.tellg();
        std::vector<char> buffer(static_cast<std::size_t>(fileSize));

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}

vko::ShaderModule::ShaderModule(Device const& device, Key key)
    : m_device(device)
    , m_key(std::move(key))
{
    std::vector<char> code = readFile(m_key.path);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

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
    stageCreateInfo.stage = getStageFlags(m_key.type);
    stageCreateInfo.module = m_handle;
    stageCreateInfo.pName = m_key.entryPoint.c_str();

    return stageCreateInfo;
}
