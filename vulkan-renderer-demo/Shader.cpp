#include "Shader.h"

#include "ShaderModule.h"
#include <iterator>

std::vector<VkPipelineShaderStageCreateInfo> vkr::Shader::createStageDescriptions() const
{
    std::vector<VkPipelineShaderStageCreateInfo> stageDescriptions;
    std::transform(m_shaderModules.begin(), m_shaderModules.end(), std::back_inserter(stageDescriptions),
                   [](ShaderModule const& shaderModule) { return shaderModule.createStageCreateInfo(); });
    return stageDescriptions;
}

vkr::Shader::Shader(Application const& app, Shader::Key const& key) : vkr::Object(app)
{
    std::vector<ShaderModule::Key> const& moduleKeys = key.getModuleKeys();
    m_shaderModules.reserve(moduleKeys.size());
	for (ShaderModule::Key const& moduleKey : moduleKeys)
		m_shaderModules.emplace_back(getApp(), moduleKey);
}
