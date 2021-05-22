#pragma once
#include <string>

#include "Object.h"
#include <vector>
#include <algorithm>
#include "ShaderModule.h"
#include <memory>

struct VkPipelineShaderStageCreateInfo;

namespace vkr
{
    class Shader : vkr::Object
    {
    public:
		class Key
		{
		public:
			std::vector<ShaderModule::Key> const& getModuleKeys() const { return m_moduleKeys; }

            Key& addStage(ShaderModule::Type type, std::string const& path, std::string entryPoint = "main")
            {
                m_moduleKeys.push_back({ type, path, entryPoint });
                return *this;
            }

		private:
			std::vector<ShaderModule::Key> m_moduleKeys;
		};

        Shader(Application const& app, Key const& key);

        std::vector<VkPipelineShaderStageCreateInfo> createStageDescriptions() const;

    private:
        std::vector<ShaderModule> m_shaderModules;
    };
}
