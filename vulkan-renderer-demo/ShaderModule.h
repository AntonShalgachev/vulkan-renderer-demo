#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <string>

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
        };

    public:
        explicit ShaderModule(Application const& app, Key const& key);
        ~ShaderModule();

        ShaderModule(ShaderModule const&) = delete;
        ShaderModule(ShaderModule&&);
        ShaderModule& operator=(ShaderModule const&) = delete;
        ShaderModule& operator=(ShaderModule&&) = delete;

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        VkShaderModule m_handle = VK_NULL_HANDLE;

        Key m_key;
    };
}

