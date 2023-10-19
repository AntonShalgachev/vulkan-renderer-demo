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

    VkExtent2D calculate_extent(tglm::ivec2 extent, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != UINT32_MAX)
            return capabilities.currentExtent;

        auto width = static_cast<uint32_t>(extent.x);
        auto height = static_cast<uint32_t>(extent.y);

        width = nstl::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        height = nstl::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return { width, height };
    }
}

// TODO move somewhere
template<>
struct picofmt::formatter<tglm::ivec2> : public picofmt::formatter<int>
{
    bool format(tglm::ivec2 const& value, context& ctx) const
    {
        ctx.write("(");
        if (!picofmt::formatter<int>::format(value.x, ctx))
            return false;
        ctx.write(",");
        if (!picofmt::formatter<int>::format(value.y, ctx))
            return false;
        ctx.write(")");

        return true;
    }
};

gfx_vk::swapchain::swapchain(context& context, gfx::renderpass_handle renderpass, surface_format surface_format, gfx::image_format depth_format, tglm::ivec2 extent)
    : m_context(context)
    , m_renderpass(renderpass)
    , m_surface_format(surface_format)
    , m_depth_format(depth_format)
{
    create(extent);
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

void gfx_vk::swapchain::resize(tglm::ivec2 extent)
{
    if (extent.x == get_extent().width && extent.y == get_extent().height)
    {
        logging::info("Skipping swapchain resize: extent is identical: {}", extent);
        return;
    }

    destroy();
    create(extent);
}

void gfx_vk::swapchain::create(tglm::ivec2 extent)
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
    config.extent = calculate_extent(extent, parameters.capabilities);

    const uint32_t min_image_count = parameters.capabilities.minImageCount;
    const uint32_t max_image_count = parameters.capabilities.maxImageCount;
    config.minImageCount = min_image_count + 1;
    if (max_image_count > 0)
        config.minImageCount = nstl::min(config.minImageCount, max_image_count);

    config.preTransform = parameters.capabilities.currentTransform;

    logging::info("Creating the swapchain with the extent {}", extent);

    m_swapchain = nstl::make_unique<vko::Swapchain>(device, m_context.get_surface(), *parameters.graphicsQueueFamily, *parameters.presentQueueFamily, nstl::move(config));
    instance.setDebugName(device.getHandle(), m_swapchain->getHandle(), "Main");

    m_depth_image = m_context.get_resources().create_image({
        .width = config.extent.width,
        .height = config.extent.height,
        .format = m_depth_format,
        .type = gfx::image_type::depth,
        .usage = gfx::image_usage::depth,
        });

    for (VkImage image : m_swapchain->getRawImages())
    {
        auto color_image = m_context.get_resources().create_image({
            .width = config.extent.width,
            .height = config.extent.height,
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
