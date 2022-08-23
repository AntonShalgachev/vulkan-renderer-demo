#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

namespace vko
{
    class Device;

    class Sampler
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
        Sampler(Device const& device);
        Sampler(Device const& device, FilterMode magFilter, FilterMode minFilter, WrapMode wrapU, WrapMode wrapV);
        ~Sampler();

        Sampler(Sampler const&) = default;
        Sampler(Sampler&&) = default;
        Sampler& operator=(Sampler const&) = default;
        Sampler& operator=(Sampler&&) = default;

        VkSampler getHandle() const { return m_handle; }

    private:
        Device const& m_device;
        UniqueHandle<VkSampler> m_handle;
    };
}
