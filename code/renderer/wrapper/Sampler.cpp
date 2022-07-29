#include "Sampler.h"
#include "Device.h"
#include <stdexcept>

namespace
{
    VkFilter vulkanizeFilterMode(vkr::Sampler::FilterMode mode)
    {
        switch (mode)
        {
        case vkr::Sampler::FilterMode::Nearest:
            return VK_FILTER_NEAREST;
        case vkr::Sampler::FilterMode::Linear:
            return VK_FILTER_LINEAR;
        }

        throw std::invalid_argument("mode");
    }

    VkSamplerAddressMode vulkanizeWrapMode(vkr::Sampler::WrapMode mode)
    {
        switch (mode)
        {
        case vkr::Sampler::WrapMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case vkr::Sampler::WrapMode::Mirror:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case vkr::Sampler::WrapMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        throw std::invalid_argument("mode");
    }
}

vkr::Sampler::Sampler(Application const& app, FilterMode magFilter, FilterMode minFilter, WrapMode wrapU, WrapMode wrapV) : Object(app)
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

    if (vkCreateSampler(getDevice().getHandle(), &samplerCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture sampler!");
}

vkr::Sampler::~Sampler()
{
    vkDestroySampler(getDevice().getHandle(), m_handle, nullptr);
}
