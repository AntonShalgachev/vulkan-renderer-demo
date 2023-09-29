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
    VkSamplerCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = get_filter_mode(params.mag_filter);
    create_info.minFilter = get_filter_mode(params.min_filter);
    create_info.addressModeU = get_wrap_mode(params.wrap_u);
    create_info.addressModeV = get_wrap_mode(params.wrap_v);
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    create_info.anisotropyEnable = VK_TRUE;
    create_info.maxAnisotropy = 16.0f;
    create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    create_info.unnormalizedCoordinates = VK_FALSE;
    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 0.0f;
    create_info.minLod = 0.0f;
    create_info.maxLod = 0.0f;

    VKO_VERIFY(vkCreateSampler(m_context.get_device().getHandle(), &create_info, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::sampler::~sampler()
{
    if (!m_handle)
        return;

    vkDestroySampler(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}
