#include "Shader.h"

#include "wrapper/ShaderModule.h"
#include <iterator>

std::vector<VkPipelineShaderStageCreateInfo> vkr::Shader::createStageDescriptions() const
{
    std::vector<VkPipelineShaderStageCreateInfo> stageDescriptions;
    std::transform(m_shaderModules.begin(), m_shaderModules.end(), std::back_inserter(stageDescriptions),
                   [](ShaderModule const& shaderModule) { return shaderModule.createStageCreateInfo(); });
    return stageDescriptions;
}

vkr::Shader::Shader(Device const& device, Shader::Key const& key) : m_device(device)
{
    std::vector<ShaderModule::Key> const& moduleKeys = key.getModuleKeys();
	if (moduleKeys.empty())
		throw std::runtime_error("Shader key is empty!");

    m_shaderModules.reserve(moduleKeys.size());
	for (ShaderModule::Key const& moduleKey : moduleKeys)
		m_shaderModules.emplace_back(m_device, moduleKey);
}
