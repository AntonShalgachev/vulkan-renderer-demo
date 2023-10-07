#include "gfx_vk/backend.h"

#include "context.h"
#include "swapchain.h"
#include "conversions.h"

#include "buffer.h"
#include "image.h"
#include "descriptor_set_layout.h"
#include "pipeline_layout.h"
#include "renderstate.h"
#include "renderpass.h"

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

gfx_vk::backend::backend(vko::Window& window, config const& config)
    : m_context(nstl::make_unique<context>(window, config))
{
    // TODO use gfx::image_format
    VkSurfaceFormatKHR vk_surface_format = find_surface_format(m_context->get_physical_device_surface_parameters().formats);
    VkFormat vk_depth_format = find_depth_format(m_context->get_physical_device());

    vko::Device const& device = m_context->get_device();

    // TODO remove
    gfx::image_format surface_format = gfx::image_format::b8g8r8a8_srgb;
    gfx::image_format depth_format = gfx::image_format::d32_float;
    assert(utils::get_format(surface_format) == vk_surface_format.format);
    assert(utils::get_format(depth_format) == vk_depth_format);

    // TODO use create_renderpass?
    m_renderpass = create_renderpass({
        .color_attachment_formats = nstl::array{ surface_format },
        .depth_stencil_attachment_format = depth_format,

        .has_presentable_images = true,
        .keep_depth_values_after_renderpass = false,
    });

    m_context->get_instance().setDebugName(device.getHandle(), m_context->get_resources().get_renderpass(m_renderpass).get_handle(), "Main renderpass");

    // disabled while the old renderer creates its own swapchain
//     m_swapchain = nstl::make_unique<swapchain>(*m_context, window, m_renderpass->get_handle(), surface_format, depth_format);

    size_t swapchain_images_count = 3; // TODO: should be retrieved from the swapchain

    for (size_t i = 0; i < swapchain_images_count; i++)
    {
        m_fake_color_images.push_back(create_image({
            .width = window.getFramebufferWidth(),
            .height = window.getFramebufferHeight(),
            .format = surface_format,
            .type = gfx::image_type::color,
            .usage = gfx::image_usage::color,
        }));
    }

    m_fake_depth_image = create_image({
        .width = window.getFramebufferWidth(),
        .height = window.getFramebufferHeight(),
        .format = depth_format,
        .type = gfx::image_type::depth,
        .usage = gfx::image_usage::depth,
    });

    for (size_t i = 0; i < swapchain_images_count; i++)
    {
        m_fake_framebuffers.push_back(create_framebuffer({
            .attachments = nstl::array{ m_fake_color_images[i], m_fake_depth_image },
            .renderpass = m_renderpass,
        }));
    }
}

gfx_vk::backend::~backend() = default;

gfx::buffer_handle gfx_vk::backend::create_buffer(gfx::buffer_params const& params)
{
    return m_context->get_resources().create_buffer(params);
}

void gfx_vk::backend::buffer_upload_sync(gfx::buffer_handle handle, nstl::blob_view bytes, size_t offset)
{
    return m_context->get_resources().get_buffer(handle).upload_sync(bytes, offset);
}

gfx::image_handle gfx_vk::backend::create_image(gfx::image_params const& params)
{
    return m_context->get_resources().create_image(params);
}

void gfx_vk::backend::image_upload_sync(gfx::image_handle handle, nstl::blob_view bytes)
{
    return m_context->get_resources().get_image(handle).upload_sync(bytes);
}

gfx::sampler_handle gfx_vk::backend::create_sampler(gfx::sampler_params const& params)
{
    return m_context->get_resources().create_sampler(params);
}

gfx::renderpass_handle gfx_vk::backend::create_renderpass(gfx::renderpass_params const& params)
{
    return m_context->get_resources().create_renderpass(params);
}

gfx::framebuffer_handle gfx_vk::backend::create_framebuffer(gfx::framebuffer_params const& params)
{
    return m_context->get_resources().create_framebuffer(params);
}

gfx::descriptorgroup_handle gfx_vk::backend::create_descriptorgroup(gfx::descriptorgroup_params const& params)
{
    return m_context->get_resources().create_descriptorgroup(params);
}

gfx::shader_handle gfx_vk::backend::create_shader(gfx::shader_params const& params)
{
    return m_context->get_resources().create_shader(params);
}

gfx::renderstate_handle gfx_vk::backend::create_renderstate(gfx::renderstate_params const& params)
{
    return m_context->get_resources().create_renderstate(params);
}

float gfx_vk::backend::get_main_framebuffer_aspect()
{
    if (!m_swapchain)
        return 1.0f * 1900 / 1000; // WTF

    VkExtent2D extent = m_swapchain->get_extent();
    return 1.0f * extent.width / extent.height;
}
