#include "Sampler.h"

#include "vko/Device.h"
#include "vko/Assert.h"
#include "vko/SamplerProperties.h"

namespace
{
    VkFilter vulkanizeFilterMode(vko::SamplerFilterMode mode)
    {
        switch (mode)
        {
        case vko::SamplerFilterMode::Nearest:
            return VK_FILTER_NEAREST;
        case vko::SamplerFilterMode::Linear:
            return VK_FILTER_LINEAR;
        }

        assert(false);
        return VK_FILTER_LINEAR;
    }

    VkSamplerAddressMode vulkanizeWrapMode(vko::SamplerWrapMode mode)
    {
        switch (mode)
        {
        case vko::SamplerWrapMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case vko::SamplerWrapMode::Mirror:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case vko::SamplerWrapMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        assert(false);
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

vko::Sampler::Sampler(Device const& device) : Sampler(device, SamplerFilterMode::Linear, SamplerFilterMode::Linear, SamplerWrapMode::Repeat, SamplerWrapMode::Repeat) {}

vko::Sampler::Sampler(Device const& device, SamplerFilterMode magFilter, SamplerFilterMode minFilter, SamplerWrapMode wrapU, SamplerWrapMode wrapV)
    : m_device(device.getHandle())
{
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = vulkanizeFilterMode(magFilter);
    samplerCreateInfo.minFilter = vulkanizeFilterMode(minFilter);
    samplerCreateInfo.addressModeU = vulkanizeWrapMode(wrapU);
    samplerCreateInfo.addressModeV = vulkanizeWrapMode(wrapV);
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;

    VKO_ASSERT(vkCreateSampler(m_device, &samplerCreateInfo, nullptr, &m_handle.get()));
}

vko::Sampler::~Sampler()
{
    vkDestroySampler(m_device, m_handle, nullptr);
}
