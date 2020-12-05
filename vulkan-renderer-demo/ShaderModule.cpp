#include "ShaderModule.h"

namespace
{
    VkShaderStageFlagBits getStageFlags(vkr::ShaderModule::Type type)
    {
        switch (type)
        {
        case vkr::ShaderModule::Type::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case vkr::ShaderModule::Type::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        throw std::domain_error("type");
    }

    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}

vkr::ShaderModule::ShaderModule(std::string const& path)
{
    std::vector<char> code = readFile(path);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(temp::getDevice(), &createInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");
}

vkr::ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(temp::getDevice(), m_handle, nullptr);
}

VkPipelineShaderStageCreateInfo vkr::ShaderModule::createStageCreateInfo(Type type, char const* entryPoint)
{
    VkPipelineShaderStageCreateInfo stageCreateInfo{};
    stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCreateInfo.stage = getStageFlags(type);
    stageCreateInfo.module = m_handle;
    stageCreateInfo.pName = entryPoint;

    return stageCreateInfo;
}
