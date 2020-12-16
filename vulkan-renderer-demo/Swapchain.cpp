#include "Swapchain.h"

#include "Renderer.h"
#include "ImageView.h"
#include "Framebuffer.h"
#include "Image.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "Device.h"
#include "Surface.h"

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

    VkExtent2D chooseSwapExtent(vkr::Surface const& surface, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        auto width = static_cast<uint32_t>(surface.getWidth());
        auto height = static_cast<uint32_t>(surface.getHeight());

        width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
        height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));

        return { width, height };
    }
}

vkr::Swapchain::Swapchain(Application const& app) : Object(app)
{
    createSwapchain();
    retrieveImages();
    createImageViews();
}

vkr::Swapchain::~Swapchain()
{
    vkDestroySwapchainKHR(getDevice().getHandle(), m_handle, nullptr);
}

void vkr::Swapchain::createFramebuffers(vkr::RenderPass const& renderPass, vkr::ImageView const& depthImageView)
{
    for (std::unique_ptr<vkr::ImageView> const& colorImageView : m_imageViews)
        m_framebuffers.push_back(std::make_unique<vkr::Framebuffer>(getApp(), *colorImageView, depthImageView, renderPass, m_extent));
}

void vkr::Swapchain::createSwapchain()
{
    PhysicalDeviceSurfaceParameters const& parameters = getPhysicalDeviceSurfaceParameters();

    m_surfaceFormat = chooseSwapSurfaceFormat(parameters.getFormats());
    VkPresentModeKHR presentMode = chooseSwapPresentMode(parameters.getPresentModes());
    m_extent = chooseSwapExtent(getSurface(), parameters.getCapabilities());

    const uint32_t minImageCount = parameters.getCapabilities().minImageCount;
    const uint32_t maxImageCount = parameters.getCapabilities().maxImageCount;

    uint32_t imageCount = minImageCount + 1;

    if (maxImageCount > 0)
        imageCount = std::min(imageCount, maxImageCount);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = getSurface().getHandle();

    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = m_surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = m_extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vkr::QueueFamilyIndices const& indices = parameters.getQueueFamilyIndices();
    if (indices.getGraphicsIndex() != indices.getPresentIndex())
    {
        uint32_t queueFamilyIndices[] = { indices.getGraphicsIndex(), indices.getPresentIndex() };
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

    swapchainCreateInfo.preTransform = parameters.getCapabilities().currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(getDevice().getHandle(), &swapchainCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create swap chain!");
}

void vkr::Swapchain::retrieveImages()
{
    uint32_t finalImageCount = 0;
    vkGetSwapchainImagesKHR(getDevice().getHandle(), m_handle, &finalImageCount, nullptr);

    std::vector<VkImage> imageHandles;
    imageHandles.resize(finalImageCount);
    vkGetSwapchainImagesKHR(getDevice().getHandle(), m_handle, &finalImageCount, imageHandles.data());

    m_images.reserve(finalImageCount);
    for (VkImage const& handle : imageHandles)
        m_images.push_back(std::make_unique<Image>(handle, m_surfaceFormat.format));
}

void vkr::Swapchain::createImageViews()
{
    m_imageViews.reserve(m_images.size());

    for (auto const& image : m_images)
        m_imageViews.push_back(image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT));
}
