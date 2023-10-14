#include "sampler.h"

#include "context.h"

#include "vko/Assert.h"
#include "vko/Device.h"

namespace
{
    VkFilter get_filter_mode(gfx::sampler_filter_mode mode)
    {
        switch (mode)
        {
        case gfx::sampler_filter_mode::nearest:
            return VK_FILTER_NEAREST;
        case gfx::sampler_filter_mode::linear:
            return VK_FILTER_LINEAR;
        }

        assert(false);
        return VK_FILTER_LINEAR;
    }

    VkSamplerAddressMode get_wrap_mode(gfx::sampler_wrap_mode mode)
    {
        switch (mode)
        {
        case gfx::sampler_wrap_mode::repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case gfx::sampler_wrap_mode::mirror:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case gfx::sampler_wrap_mode::clamp_to_edge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        assert(false);
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

gfx_vk::sampler::sampler(context& context, gfx::sampler_params const& params)
    : m_context(context)
{
    VkSamplerCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = get_filter_mode(params.mag_filter),
        .minFilter = get_filter_mode(params.min_filter),
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = get_wrap_mode(params.wrap_u),
        .addressModeV = get_wrap_mode(params.wrap_v),
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 16.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VKO_VERIFY(vkCreateSampler(m_context.get_device().getHandle(), &info, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::sampler::~sampler()
{
    if (!m_handle)
        return;

    vkDestroySampler(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}
