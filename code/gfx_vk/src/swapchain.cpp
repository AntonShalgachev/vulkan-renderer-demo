#include "swapchain.h"

#include "context.h"
#include "conversions.h"

#include "renderpass.h"

#include "vko/Device.h"
#include "vko/DeviceMemory.h"
#include "vko/Framebuffer.h"
#include "vko/Instance.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"
#include "vko/Swapchain.h"
#include "vko/Window.h"

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

    VkExtent2D calculate_extent(vko::Window const& window, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;

        auto width = static_cast<uint32_t>(window.getFramebufferWidth());
        auto height = static_cast<uint32_t>(window.getFramebufferHeight());

        width = nstl::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        height = nstl::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return { width, height };
    }
}

gfx_vk::swapchain::swapchain(context& context, vko::Window& window, gfx::renderpass_handle renderpass, surface_format surface_format, gfx::image_format depth_format)
    : m_context(context)
    , m_window(window)
    , m_renderpass(renderpass)
    , m_surface_format(surface_format)
    , m_depth_format(depth_format)
{
    m_window.addFramebufferResizeCallback([this](int, int) { on_window_resized(); });

    recreate();
}

gfx_vk::swapchain::~swapchain()
{
    destroy();
}

VkSwapchainKHR gfx_vk::swapchain::get_handle() const
{
    return m_swapchain->getHandle();
}

VkExtent2D gfx_vk::swapchain::get_extent() const
{
    return m_swapchain->getExtent();
}

void gfx_vk::swapchain::on_window_resized()
{
    m_window.waitUntilInForeground(); // TODO remove from here
    m_context.on_surface_changed();
    recreate();
}

void gfx_vk::swapchain::create()
{
    vko::Instance const& instance = m_context.get_instance();
    vko::Device const& device = m_context.get_device();

    vko::PhysicalDeviceSurfaceParameters const& parameters = m_context.get_physical_device_surface_parameters();

    vko::Swapchain::Config config;
    config.surfaceFormat = {
        .format = utils::get_format(m_surface_format.format),
        .colorSpace = utils::get_color_space(m_surface_format.color_space),
    };
    config.presentMode = find_present_mode(parameters.presentModes);
    config.extent = calculate_extent(m_window, parameters.capabilities);

    const uint32_t min_image_count = parameters.capabilities.minImageCount;
    const uint32_t max_image_count = parameters.capabilities.maxImageCount;
    config.minImageCount = min_image_count + 1;
    if (max_image_count > 0)
        config.minImageCount = nstl::min(config.minImageCount, max_image_count);

    config.preTransform = parameters.capabilities.currentTransform;

    m_swapchain = nstl::make_unique<vko::Swapchain>(device, m_context.get_surface(), *parameters.graphicsQueueFamily, *parameters.presentQueueFamily, nstl::move(config));
    instance.setDebugName(device.getHandle(), m_swapchain->getHandle(), "Main");

    VkExtent2D extent = m_swapchain->getExtent();

    m_depth_image = m_context.get_resources().create_image({
        .width = extent.width,
        .height = extent.height,
        .format = m_depth_format,
        .type = gfx::image_type::depth,
        .usage = gfx::image_usage::depth,
    });

    for (VkImage image : m_swapchain->getRawImages())
    {
        auto color_image = m_context.get_resources().create_image({
            .width = extent.width,
            .height = extent.height,
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
    m_context.get_device().waitIdle();

    m_framebuffers.clear();
    // TODO destroy m_swapchain_images
    // TODO destroy m_depth_image
    m_swapchain = nullptr;
}

void gfx_vk::swapchain::recreate()
{
    destroy();
    create();
}
