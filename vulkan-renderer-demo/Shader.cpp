#include "Shader.h"

#include "ShaderModule.h"

std::vector<VkPipelineShaderStageCreateInfo> vkr::CompiledShader::createStageDescriptions() const
{
    std::vector<VkPipelineShaderStageCreateInfo> stageDescriptions;
    std::transform(m_shaderModules.begin(), m_shaderModules.end(), std::back_inserter(stageDescriptions),
                   [](ShaderModule const& shaderModule) { return shaderModule.createStageCreateInfo(); });
    return stageDescriptions;
}

vkr::Shader::Shader(Application const& app, std::vector<ShaderModule::Key> moduleKeys) : Object(app), m_moduleKeys(std::move(moduleKeys))
{

}

vkr::CompiledShader vkr::Shader::compile() const
{
    std::vector<ShaderModule> modules;
    for (ShaderModule::Key const& key : m_moduleKeys)
        modules.emplace_back(getApp(), key);
    return CompiledShader(std::move(modules));
}

vkr::ShaderBuilder& vkr::ShaderBuilder::addStage(ShaderModule::Type type, std::string const& path, std::string entryPoint)
{
    m_moduleKeys.push_back({ type, path, entryPoint });
    return *this;
}

std::unique_ptr<vkr::Shader> vkr::ShaderBuilder::build(Application const& app) const
{
    return std::make_unique<vkr::Shader>(app, m_moduleKeys);
}
