#include "Swapchain.h"

#include "vko/Assert.h"
#include "vko/Image.h"
#include "vko/Device.h"
#include "vko/Surface.h"

vko::Swapchain::Swapchain(Device const& device, Surface const& surface, uint32_t graphics, uint32_t presentation, Config config)
    : m_device(device)
    , m_config(nstl::move(config))
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

    if (graphics != presentation)
    {
        uint32_t queueFamilyIndices[] = { graphics, presentation };
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
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // TODO use it when recreating the swapchain

    VKO_VERIFY(vkCreateSwapchainKHR(m_device.getHandle(), &swapchainCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));

    retrieveImages();
}

vko::Swapchain::~Swapchain()
{
    vkDestroySwapchainKHR(m_device.getHandle(), m_handle, &m_allocator.getCallbacks());
}

VkExtent2D vko::Swapchain::getExtent() const
{
    return m_config.extent;
}

VkSurfaceFormatKHR vko::Swapchain::getSurfaceFormat() const
{
    return m_config.surfaceFormat;
}

void vko::Swapchain::retrieveImages()
{
    uint32_t finalImageCount = 0;
    VKO_VERIFY(vkGetSwapchainImagesKHR(m_device.getHandle(), m_handle, &finalImageCount, nullptr));

    m_rawImages.resize(finalImageCount);
    VKO_VERIFY(vkGetSwapchainImagesKHR(m_device.getHandle(), m_handle, &finalImageCount, m_rawImages.data()));
}
