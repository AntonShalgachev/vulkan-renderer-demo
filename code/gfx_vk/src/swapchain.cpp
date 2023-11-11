#include "swapchain.h"

#include "context.h"
#include "conversions.h"

#include "renderpass.h"

#include "logging/logging.h"

#include "nstl/algorithm.h"
#include "nstl/array.h"
#include "nstl/sprintf.h"

namespace
{
    VkPresentModeKHR find_present_mode(nstl::span<VkPresentModeKHR const> availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D calculate_extent(size_t w, size_t h, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != UINT32_MAX)
            return capabilities.currentExtent;

        auto width = static_cast<uint32_t>(w);
        auto height = static_cast<uint32_t>(h);

        width = nstl::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        height = nstl::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return { width, height };
    }
}

gfx_vk::swapchain::swapchain(context& context, gfx::renderpass_handle renderpass, surface_format surface_format, gfx::image_format depth_format, size_t w, size_t h)
    : m_context(context)
    , m_renderpass(renderpass)
    , m_surface_format(surface_format)
    , m_depth_format(depth_format)
{
    create(w, h);
}

gfx_vk::swapchain::~swapchain()
{
    destroy();
}

VkSwapchainKHR gfx_vk::swapchain::get_handle() const
{
    return m_handle;
}

VkExtent2D gfx_vk::swapchain::get_extent() const
{
    return m_extent;
}

void gfx_vk::swapchain::resize(size_t w, size_t h)
{
    if (static_cast<uint32_t>(w) == m_extent.width && static_cast<uint32_t>(h) == m_extent.height)
    {
        logging::info("Skipping swapchain resize: extent is identical: ({}, {})", w, h);
        return;
    }

    destroy();
    create(w, h);
}

void gfx_vk::swapchain::create(size_t w, size_t h)
{
    physical_device_properties const& parameters = m_context.get_physical_device_props();

    uint32_t const min_image_count = parameters.capabilities.minImageCount;
    uint32_t const max_image_count = parameters.capabilities.maxImageCount;
    uint32_t image_count = min_image_count + 1;
    if (max_image_count > 0)
        image_count = nstl::min(image_count, max_image_count);

    logging::info("Creating the swapchain with the extent ({}, {})", w, h);

    assert(m_handle == VK_NULL_HANDLE);

    m_extent = calculate_extent(w, h, parameters.capabilities);

    VkSwapchainCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_context.get_surface_handle(),

        .minImageCount = image_count,
        .imageFormat = utils::get_format(m_surface_format.format),
        .imageColorSpace = utils::get_color_space(m_surface_format.color_space),
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

        .preTransform = parameters.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = find_present_mode(parameters.present_modes),
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE, // TODO use
    };

    if (*parameters.graphics_queue_family != *parameters.present_queue_family)
    {
        uint32_t queueFamilyIndices[] = { *parameters.graphics_queue_family, *parameters.present_queue_family };
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;
    }

    GFX_VK_VERIFY(vkCreateSwapchainKHR(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));

    uint32_t count = 0;
    GFX_VK_VERIFY(vkGetSwapchainImagesKHR(m_context.get_device_handle(), m_handle, &count, nullptr));

    m_images.resize(count);
    GFX_VK_VERIFY(vkGetSwapchainImagesKHR(m_context.get_device_handle(), m_handle, &count, m_images.data()));

    m_context.get_instance().set_debug_name(m_handle, "Main swapchain with extent ({}, {})", w, h);

    m_depth_image = m_context.get_resources().create_image({
        .width = m_extent.width,
        .height = m_extent.height,
        .format = m_depth_format,
        .type = gfx::image_type::depth,
        .usage = gfx::image_usage::depth,
    });

    for (VkImage image : m_images)
    {
        auto color_image = m_context.get_resources().create_image({
            .width = m_extent.width,
            .height = m_extent.height,
            .format = m_surface_format.format,
            .type = gfx::image_type::color,
            .usage = gfx::image_usage::color,
        }, image);

        auto framebuffer = m_context.get_resources().create_framebuffer({
            .attachments = nstl::array{color_image, m_depth_image},
            .renderpass = m_renderpass,
        });

        m_swapchain_images.push_back(color_image);
        m_framebuffers.push_back(framebuffer);
    }
}

void gfx_vk::swapchain::destroy()
{
    m_context.get_instance().wait_idle();

    m_framebuffers.clear();
    // TODO destroy m_swapchain_images
    // TODO destroy m_depth_image
    
    vkDestroySwapchainKHR(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
    m_handle = VK_NULL_HANDLE;
}
