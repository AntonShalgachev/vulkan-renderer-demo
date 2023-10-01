#include "gfx_vk/backend.h"

#include "context.h"
#include "swapchain.h"

#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include "shader.h"
#include "descriptor_set_layout.h"
#include "pipeline_layout.h"
#include "renderstate.h"
#include "renderpass.h"
#include "conversions.h"

#include "vko/Device.h"
#include "vko/Instance.h"
#include "vko/PhysicalDevice.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"
#include "vko/Window.h"

#include "nstl/array.h"

namespace
{
    VkFormat find_supported_format(vko::PhysicalDevice const& physicalDevice, nstl::span<VkFormat const> candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice.getHandle(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }

        assert(false);
        return VkFormat{};
    }

    VkFormat find_depth_format(vko::PhysicalDevice const& physicalDevice)
    {
        VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        return find_supported_format(physicalDevice, candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkSurfaceFormatKHR find_surface_format(nstl::span<VkSurfaceFormatKHR const> surface_formats)
    {
        if (surface_formats.empty())
            return { VK_FORMAT_UNDEFINED , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (const auto& surface_format : surface_formats)
            if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return surface_format;

        return surface_formats[0];
    }
}

gfx_vk::backend::backend(vko::Window& window, char const* name, bool enable_validation)
{
    m_context = nstl::make_unique<context>(window, name, enable_validation);

    // TODO use gfx::image_format
    VkSurfaceFormatKHR surface_format = find_surface_format(m_context->get_physical_device_surface_parameters().formats);
    VkFormat depth_format = find_depth_format(m_context->get_physical_device());

    vko::Device const& device = m_context->get_device();

    // TODO remove hardcoded values are removed
    assert(utils::get_format(gfx::image_format::b8g8r8a8_srgb) == surface_format.format);
    assert(utils::get_format(gfx::image_format::d32_float) == depth_format);

    // TODO use create_renderpass?
    m_renderpass = nstl::make_unique<renderpass>(*m_context, gfx::renderpass_params{
        .color_attachment_formats = nstl::array{ gfx::image_format::b8g8r8a8_srgb },
        .depth_stencil_attachment_format = gfx::image_format::d32_float,

        .has_presentable_images = true,
        .keep_depth_values_after_renderpass = false,
    });

    m_context->get_instance().setDebugName(device.getHandle(), m_renderpass->get_handle(), "Main renderpass");

    // disabled while the old renderer creates its own swapchain
//     m_swapchain = nstl::make_unique<swapchain>(*m_context, window, m_renderpass->get_handle(), surface_format, depth_format);
}

gfx_vk::backend::~backend() = default;

nstl::unique_ptr<gfx::buffer> gfx_vk::backend::create_buffer(gfx::buffer_params const& params)
{
    return nstl::make_unique<buffer>(*m_context, params);
}

nstl::unique_ptr<gfx::image> gfx_vk::backend::create_image(gfx::image_params const& params)
{
    return nstl::make_unique<image>(*m_context, params);
}

nstl::unique_ptr<gfx::sampler> gfx_vk::backend::create_sampler(gfx::sampler_params const& params)
{
    return nstl::make_unique<sampler>(*m_context, params);
}

nstl::unique_ptr<gfx::renderpass> gfx_vk::backend::create_renderpass(gfx::renderpass_params const& params)
{
    return nstl::make_unique<renderpass>(*m_context, params);
}

nstl::unique_ptr<gfx::shader> gfx_vk::backend::create_shader(gfx::shader_params const& params)
{
    return nstl::make_unique<shader>(*m_context, params);
}

nstl::unique_ptr<gfx::renderstate> gfx_vk::backend::create_renderstate(gfx::renderstate_params const& params)
{
    // WTF: implement properly

    nstl::vector<VkDescriptorSetLayout> set_layout_handles;

    for (gfx::uniform_group_configuration const& p : params.uniform_groups_config)
    {
        auto set_layout = nstl::make_unique<descriptor_set_layout>(*m_context, p);
        set_layout_handles.push_back(set_layout->get_handle());

        m_descriptor_set_layouts.push_back(nstl::move(set_layout));
    }

    auto layout = nstl::make_unique<pipeline_layout>(*m_context, set_layout_handles);
    VkPipelineLayout layout_handle = layout->get_handle();
    m_pipeline_layouts.push_back(nstl::move(layout));

    renderstate_init_params init_params = {
        .shaders = params.shaders,
        .vertex_config = params.vertex_config,
        .flags = params.flags,

        .layout = layout_handle,
        .renderpass = static_cast<renderpass const*>(params.renderpass)->get_handle(),
    };

    return nstl::make_unique<renderstate>(*m_context, init_params);
}

gfx::renderpass* gfx_vk::backend::get_main_renderpass()
{
    return m_renderpass.get();
}
