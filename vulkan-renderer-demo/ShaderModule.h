#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <string>
#include "UniqueHandle.h"

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

        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(ShaderModule&&) = default;

        VkPipelineShaderStageCreateInfo createStageCreateInfo() const;

        VkShaderModule getHandle() const { return m_handle; }

    private:
        UniqueHandle<VkShaderModule> m_handle;

        Key m_key;
    };
}

