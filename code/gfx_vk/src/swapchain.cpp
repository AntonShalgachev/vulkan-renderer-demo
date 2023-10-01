#include "swapchain.h"

#include "context.h"

#include "vko/Device.h"
#include "vko/DeviceMemory.h"
#include "vko/Framebuffer.h"
#include "vko/Image.h"
#include "vko/ImageView.h"
#include "vko/Instance.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"
#include "vko/Swapchain.h"
#include "vko/Window.h"

#include "nstl/algorithm.h"
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

gfx_vk::swapchain::swapchain(context& context, vko::Window& window, VkRenderPass render_pass, VkSurfaceFormatKHR surface_format, VkFormat depth_format)
    : m_context(context)
    , m_window(window)
    , m_renderpass(render_pass)
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

void gfx_vk::swapchain::on_window_resized()
{
    recreate();
}

void gfx_vk::swapchain::create()
{
    vko::Instance const& instance = m_context.get_instance();
    vko::Device const& device = m_context.get_device();

    vko::PhysicalDeviceSurfaceParameters const& parameters = m_context.get_physical_device_surface_parameters();

    vko::Swapchain::Config config;
    config.surfaceFormat = m_surface_format;
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

    // TODO move swapchain images to the ResourceManager?

    auto const& images = m_swapchain->getImages();
    m_image_views.reserve(images.size());

    for (size_t i = 0; i < images.size(); i++)
    {
        vko::Image const& image = images[i];
        m_image_views.push_back(nstl::make_unique<vko::ImageView>(device, image, VK_IMAGE_ASPECT_COLOR_BIT));

        instance.setDebugName(device.getHandle(), image.getHandle(), nstl::sprintf("Swapchain image %zu", i));
        instance.setDebugName(device.getHandle(), m_image_views.back()->getHandle(), nstl::sprintf("Swapchain image view %zu", i));
    }

    VkExtent2D extent = m_swapchain->getExtent();

    m_depth_image = nstl::make_unique<vko::Image>(device, extent.width, extent.height, m_depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depth_image_memory = nstl::make_unique<vko::DeviceMemory>(device, m_context.get_physical_device(), m_depth_image->getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_depth_image->bindMemory(*m_depth_image_memory);

    m_depth_image_view = nstl::make_unique<vko::ImageView>(device, *m_depth_image, VK_IMAGE_ASPECT_DEPTH_BIT);

    instance.setDebugName(device.getHandle(), m_depth_image->getHandle(), "Main depth image");
    instance.setDebugName(device.getHandle(), m_depth_image_view->getHandle(), "Main depth image view");

    m_framebuffers.reserve(m_image_views.size());
    for (size_t i = 0; i < m_image_views.size(); i++)
    {
        nstl::unique_ptr<vko::ImageView> const& color_image_view = m_image_views[i];
        m_framebuffers.push_back(nstl::make_unique<vko::Framebuffer>(device, color_image_view.get(), m_depth_image_view.get(), m_renderpass, m_swapchain->getExtent()));

        instance.setDebugName(device.getHandle(), m_framebuffers.back()->getHandle(), nstl::sprintf("Main framebuffer %zu", i));
    }
}

void gfx_vk::swapchain::destroy()
{
    m_context.get_device().waitIdle();

    m_framebuffers.clear();
    m_depth_image_view = nullptr;
    m_depth_image = nullptr;
    m_depth_image_memory = nullptr;
    m_image_views.clear();
    m_swapchain = nullptr;
}

void gfx_vk::swapchain::recreate()
{
    destroy();
    create();
}
