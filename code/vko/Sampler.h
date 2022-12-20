#pragma once

#include "vko/UniqueHandle.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Device;

    enum class SamplerFilterMode;
    enum class SamplerWrapMode;

    class Sampler
    {
    public:
        Sampler(Device const& device);
        Sampler(Device const& device, SamplerFilterMode magFilter, SamplerFilterMode minFilter, SamplerWrapMode wrapU, SamplerWrapMode wrapV);
        ~Sampler();

        Sampler(Sampler const&) = default;
        Sampler(Sampler&&) = default;
        Sampler& operator=(Sampler const&) = default;
        Sampler& operator=(Sampler&&) = default;

        VkSampler getHandle() const { return m_handle; }

    private:
        VkDevice m_device;
        UniqueHandle<VkSampler> m_handle;
    };
}
