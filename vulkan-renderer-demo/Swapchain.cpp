#include "Swapchain.h"

#include "Renderer.h"
#include "ImageView.h"
#include "Framebuffer.h"

namespace
{
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.empty())
            return { VK_FORMAT_UNDEFINED , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        auto width = static_cast<uint32_t>(vkr::temp::getRenderer()->getWidth());
        auto height = static_cast<uint32_t>(vkr::temp::getRenderer()->getHeight());

        width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
        height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));

        return { width, height };
    }
}

vkr::Swapchain::Swapchain()
{
    createSwapchain();
    retrieveImages();
    createImageViews();
}

vkr::Swapchain::~Swapchain()
{
    vkDestroySwapchainKHR(temp::getDevice(), m_handle, nullptr);
}

void vkr::Swapchain::createFramebuffers(vkr::RenderPass const& renderPass, vkr::ImageView const& depthImageView)
{
    for (std::unique_ptr<vkr::ImageView> const& colorImageView : m_imageViews)
        m_framebuffers.push_back(std::make_unique<vkr::Framebuffer>(*colorImageView, depthImageView, renderPass, m_extent));
}

void vkr::Swapchain::createSwapchain()
{
    vkr::Renderer::SwapchainSupportDetails swapChainSupport = temp::getRenderer()->getPhysicalDeviceProperties().swapchainSupportDetails;

    m_surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    m_extent = chooseSwapExtent(swapChainSupport.capabilities);

    const uint32_t minImageCount = swapChainSupport.capabilities.minImageCount;
    const uint32_t maxImageCount = swapChainSupport.capabilities.maxImageCount;

    uint32_t imageCount = minImageCount + 1;

    if (maxImageCount > 0)
        imageCount = std::min(imageCount, maxImageCount);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = temp::getRenderer()->getSurfaceHandle();

    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = m_surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = m_extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vkr::Renderer::QueueFamilyIndices indices = temp::getRenderer()->getPhysicalDeviceProperties().queueFamilyIndices;
    if (indices.graphicsFamily != indices.presentFamily)
    {
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
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

    swapchainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(temp::getDevice(), &swapchainCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");
}

void vkr::Swapchain::retrieveImages()
{
    uint32_t finalImageCount = 0;
    vkGetSwapchainImagesKHR(temp::getDevice(), m_handle, &finalImageCount, nullptr);
    m_images.resize(finalImageCount);
    vkGetSwapchainImagesKHR(temp::getDevice(), m_handle, &finalImageCount, m_images.data());
}

void vkr::Swapchain::createImageViews()
{
    m_imageViews.resize(m_images.size());

    for (std::size_t i = 0; i < m_images.size(); i++)
        m_imageViews[i] = std::make_unique<vkr::ImageView>(m_images[i], m_surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
}
