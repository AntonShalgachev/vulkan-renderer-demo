#pragma once
#include <string>

#include "Object.h"
#include <vector>
#include <algorithm>
#include "ShaderModule.h"

struct VkPipelineShaderStageCreateInfo;

namespace vkr
{
    class CompiledShader
    {
    public:
        CompiledShader(std::vector<ShaderModule> shaderModules) : m_shaderModules(std::move(shaderModules)) {}
        std::vector<VkPipelineShaderStageCreateInfo> createStageDescriptions() const;

    private:
        std::vector<ShaderModule> m_shaderModules;
    };

    class Shader : vkr::Object
    {
    public:
    	Shader(Application const& app, std::vector<ShaderModule::Key> moduleKeys);

        CompiledShader compile() const;

    private:
        std::vector<ShaderModule::Key> m_moduleKeys;
    };

    class ShaderBuilder
    {
    public:
        ShaderBuilder& addStage(ShaderModule::Type type, std::string const& path, std::string entryPoint = "main");
        std::unique_ptr<Shader> build(Application const& app) const;

    private:
        std::vector<ShaderModule::Key> m_moduleKeys;
    };
}
