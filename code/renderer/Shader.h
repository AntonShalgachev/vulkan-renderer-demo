#pragma once
#include <string>

#include <vector>
#include <algorithm>
#include "wrapper/ShaderModule.h"
#include <memory>
#include "Hash.h"

struct VkPipelineShaderStageCreateInfo;

namespace vkr
{
	class Device;

    class Shader
    {
    public:
		class Key
		{
		public:
			std::vector<vko::ShaderModule::Key> const& getModuleKeys() const { return m_moduleKeys; }

            Key& addStage(vko::ShaderModule::Type type, std::string path, std::string entryPoint = "main")
            {
                m_moduleKeys.push_back({ type, std::move(path), std::move(entryPoint) });
                return *this;
            }

			std::size_t computeHash() const
			{
				std::size_t seed = 0;
				vkr::hash::combine(seed, m_moduleKeys);
				return seed;
			}

			auto operator<=>(Key const& rhs) const = default;

		private:
			std::vector<vko::ShaderModule::Key> m_moduleKeys;
		};

		Shader(vko::Device const& device, Key const& key);

		std::vector<vko::ShaderModule> const& getModules() const { return m_shaderModules; }

	private:
		vko::Device const& m_device;
		std::vector<vko::ShaderModule> m_shaderModules;
    };
}

namespace std
{
	template<>
	struct hash<vkr::Shader::Key>
	{
		std::size_t operator()(vkr::Shader::Key const& rhs) const
		{
			return rhs.computeHash();
		}
	};
}
