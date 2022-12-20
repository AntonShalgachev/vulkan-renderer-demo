#include "Swapchain.h"

#include "vko/Assert.h"
#include "vko/Image.h"
#include "vko/Device.h"
#include "vko/Surface.h"
#include "vko/QueueFamily.h"

vko::Swapchain::Swapchain(Device const& device, Surface const& surface, QueueFamily const& graphics, QueueFamily const& presentation, Config config)
    : m_device(device)
    , m_config(std::move(config))
{
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface.getHandle();

    swapchainCreateInfo.minImageCount = config.minImageCount;
    swapchainCreateInfo.imageFormat = config.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = config.surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = config.extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (graphics.getIndex() != presentation.getIndex())
    {
        uint32_t queueFamilyIndices[] = { graphics.getIndex(), presentation.getIndex() };
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = config.preTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = config.presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VKO_ASSERT(vkCreateSwapchainKHR(m_device.getHandle(), &swapchainCreateInfo, nullptr, &m_handle.get()));

    retrieveImages();
}

vko::Swapchain::~Swapchain()
{
    vkDestroySwapchainKHR(m_device.getHandle(), m_handle, nullptr);
}

VkExtent2D vko::Swapchain::getExtent() const
{
    return m_config.extent;
}

VkSurfaceFormatKHR vko::Swapchain::getSurfaceFormat() const
{
    return m_config.surfaceFormat;
}

std::size_t vko::Swapchain::getImageCount() const
{
    return m_images.size();
}

nstl::vector<vko::Image> const& vko::Swapchain::getImages() const
{
    return m_images;
}

void vko::Swapchain::retrieveImages()
{
    uint32_t finalImageCount = 0;
    VKO_ASSERT(vkGetSwapchainImagesKHR(m_device.getHandle(), m_handle, &finalImageCount, nullptr));

    nstl::vector<VkImage> imageHandles;
    imageHandles.resize(finalImageCount);
    VKO_ASSERT(vkGetSwapchainImagesKHR(m_device.getHandle(), m_handle, &finalImageCount, imageHandles.data()));

    m_images.reserve(finalImageCount);
    for (VkImage const& handle : imageHandles)
        m_images.emplace_back(m_device, handle, m_config.surfaceFormat.format);
}
