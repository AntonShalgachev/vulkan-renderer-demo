#include "Sampler.h"
#include "Device.h"
#include <stdexcept>

namespace
{
    VkFilter vulkanizeFilterMode(vko::Sampler::FilterMode mode)
    {
        switch (mode)
        {
        case vko::Sampler::FilterMode::Nearest:
            return VK_FILTER_NEAREST;
        case vko::Sampler::FilterMode::Linear:
            return VK_FILTER_LINEAR;
        }

        throw std::invalid_argument("mode");
    }

    VkSamplerAddressMode vulkanizeWrapMode(vko::Sampler::WrapMode mode)
    {
        switch (mode)
        {
        case vko::Sampler::WrapMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case vko::Sampler::WrapMode::Mirror:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case vko::Sampler::WrapMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        throw std::invalid_argument("mode");
    }
}

vko::Sampler::Sampler(Device const& device) : Sampler(device, FilterMode::Linear, FilterMode::Linear, WrapMode::Repeat, WrapMode::Repeat) {}

vko::Sampler::Sampler(Device const& device, FilterMode magFilter, FilterMode minFilter, WrapMode wrapU, WrapMode wrapV)
    : m_device(device)
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

    if (vkCreateSampler(m_device.getHandle(), &samplerCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture sampler!");
}

vko::Sampler::~Sampler()
{
    vkDestroySampler(m_device.getHandle(), m_handle, nullptr);
}
