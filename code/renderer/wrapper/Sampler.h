#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class Sampler : public Object
    {
    public:
        enum class FilterMode
        {
            Nearest,
            Linear,
        };

        enum class WrapMode
        {
            Repeat,
            Mirror,
            ClampToEdge,
        };

    public:
        Sampler(Application const& app);
        Sampler(Application const& app, FilterMode magFilter, FilterMode minFilter, WrapMode wrapU, WrapMode wrapV);
        ~Sampler();

        Sampler(Sampler const&) = default;
        Sampler(Sampler&&) = default;
        Sampler& operator=(Sampler const&) = default;
        Sampler& operator=(Sampler&&) = default;

        VkSampler getHandle() const { return m_handle; }

    private:
        UniqueHandle<VkSampler> m_handle;
    };
}
