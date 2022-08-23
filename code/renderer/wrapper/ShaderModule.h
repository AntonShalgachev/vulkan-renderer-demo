#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "UniqueHandle.h"
#include "Hash.h"

namespace vko
{
    class Device;

    class ShaderModule
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

            auto operator<=>(Key const& rhs) const = default;

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
        explicit ShaderModule(Device const& device, Key key);
        ~ShaderModule();

        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(ShaderModule&&) = default;

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        Device const& m_device;
        Key m_key;

        UniqueHandle<VkShaderModule> m_handle;
    };
}

namespace std
{
	template<>
	struct hash<vko::ShaderModule::Key>
	{
		std::size_t operator()(vko::ShaderModule::Key const& rhs) const
		{
			return rhs.computeHash();
		}
	};
}
