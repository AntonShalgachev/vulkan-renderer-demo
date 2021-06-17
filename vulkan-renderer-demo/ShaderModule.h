#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <string>
#include "UniqueHandle.h"
#include "Hash.h"

namespace vkr
{
    class ShaderModule : Object
    {
    public:
        enum class Type
        {
            Vertex,
            Geometry,
            Fragment,
        };

        struct Key
        {
            Type type = Type::Vertex;
            std::string path;
            std::string entryPoint;

			std::size_t computeHash() const
			{
				std::size_t seed = 0;
                vkr::hash::combine(seed, type);
                vkr::hash::combine(seed, path);
                vkr::hash::combine(seed, entryPoint);
				return seed;
			}
        };

    public:
        explicit ShaderModule(Application const& app, Key const& key);
        ~ShaderModule();

        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(ShaderModule&&) = default;

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        UniqueHandle<VkShaderModule> m_handle;

        Key m_key;
    };
}

namespace std
{
	template<>
	struct hash<vkr::ShaderModule::Key>
	{
		std::size_t operator()(vkr::ShaderModule::Key const& rhs) const
		{
			return rhs.computeHash();
		}
	};
}
