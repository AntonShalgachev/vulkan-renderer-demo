#include "Renderer.h"

#include <array>
#include <stdexcept>

#include "glm.h"

#include "Application.h"
#include "wrapper/Device.h"
#include "wrapper/CommandBuffers.h"
#include "wrapper/Queue.h"
#include "wrapper/Swapchain.h"
#include "wrapper/RenderPass.h"
#include "wrapper/Framebuffer.h"
#include "wrapper/Pipeline.h"
#include "wrapper/Image.h"
#include "wrapper/ImageView.h"
#include "wrapper/DeviceMemory.h"
#include "wrapper/PipelineLayout.h"
#include "Utils.h"
#include "wrapper/ShaderModule.h"
#include "Mesh.h"
#include "ObjectInstance.h"
#include "Drawable.h"
#include "wrapper/DescriptorSetLayout.h"
#include "wrapper/Buffer.h"
#include "Material.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "wrapper/CommandPool.h"
#include "CommandBuffer.h"
#include "SceneObject.h"

#pragma warning(push, 0)
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#pragma warning(pop)

#include "VertexLayout.h"
#include "PipelineConfiguration.h"
#include "wrapper/Sampler.h"
#include "Texture.h"
#include "BufferWithMemory.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Surface.h"
#include "ScopedOneTimeCommandBuffer.h"
#include "wrapper/DescriptorPool.h"

namespace
{
    struct ObjectUniformBuffer
    {
        glm::vec4 objectColor;
    };

    struct MaterialUniformBuffer
    {
        glm::vec4 objectColor;
    };

    struct FrameUniformBuffer
    {
        glm::mat4 projection;
        glm::vec3 lightPosition;
        glm::vec3 lightColor;
    };

    struct VertexPushConstants
    {
        glm::mat4 modelView;
        glm::mat4 normal;
    };

    const int FRAME_RESOURCE_COUNT = 3;

    VkFormat findSupportedFormat(vko::PhysicalDevice const& physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat findDepthFormat(vko::PhysicalDevice const& physicalDevice)
    {
        static const std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

        return findSupportedFormat(physicalDevice, candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

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

    VkExtent2D chooseSwapExtent(vko::Surface const& surface, const VkSurfaceCapabilitiesKHR& capabilities)
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

// TODO move somewhere
namespace
{
    // normals: offset 0, stride 24, type vec3, count 24
    // position: offset 12, stride 24, type vec3, count 24
    // indices: offset 576, type unsigned short, count 36
    static std::vector<unsigned char> const boxBuffer = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x3f,
        0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0xbf,
        0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04, 0x00, 0x05, 0x00,
        0x06, 0x00, 0x07, 0x00, 0x06, 0x00, 0x05, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0a, 0x00, 0x0b, 0x00,
        0x0a, 0x00, 0x09, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x0e, 0x00, 0x0f, 0x00, 0x0e, 0x00, 0x0d, 0x00,
        0x10, 0x00, 0x11, 0x00, 0x12, 0x00, 0x13, 0x00, 0x12, 0x00, 0x11, 0x00, 0x14, 0x00, 0x15, 0x00,
        0x16, 0x00, 0x17, 0x00, 0x16, 0x00, 0x15, 0x00
    };

    struct OneTimeGeometryPushConstants
    {
        glm::mat4 modelViewProjection;
    };

    vkr::VertexLayout createBoxVertexLayout()
    {
        vkr::VertexLayout layout;

        layout.setIndexType(vkr::VertexLayout::ComponentType::UnsignedShort);
        layout.setIndexDataOffset(576);
        layout.setIndexCount(36);

        std::vector<vkr::VertexLayout::Binding> bindings;

        bindings.emplace_back(12, 6 * sizeof(float)).addAttribute(0, vkr::VertexLayout::AttributeType::Vec3, vkr::VertexLayout::ComponentType::Float, 0);
        bindings.emplace_back(0, 6 * sizeof(float)).addAttribute(1, vkr::VertexLayout::AttributeType::Vec3, vkr::VertexLayout::ComponentType::Float, 0);

        layout.setBindings(bindings);

        return layout;
    }

    // TODO unify with DemoApplication::loadScene
    std::unique_ptr<vkr::BufferWithMemory> createBufferAndMemory(vkr::Application const& app, std::vector<unsigned char> const& bytes)
    {
        VkDeviceSize bufferSize = sizeof(bytes[0]) * bytes.size();
        void const* bufferData = bytes.data();

        vkr::BufferWithMemory stagingBuffer{ app.getDevice(), app.getPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        stagingBuffer.memory().copyFrom(bufferData, bufferSize);

        vkr::BufferWithMemory buffer{ app.getDevice(), app.getPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

        vkr::ScopedOneTimeCommandBuffer commandBuffer{ app };
        vko::Buffer::copy(commandBuffer.getHandle(), stagingBuffer.buffer(), buffer.buffer());
        commandBuffer.submit();

        return std::make_unique<vkr::BufferWithMemory>(std::move(buffer));
    }
}

vkr::Renderer::Renderer(Application const& app)
    : Object(app)
{
    createSwapchain();
    createSyncObjects();
}

vkr::Renderer::~Renderer()
{
    destroySwapchain(); // TODO hack: this ensures the right order
}

void vkr::Renderer::setWaitUntilWindowInForegroundCallback(std::function<void()> func)
{
    m_waitUntilWindowInForeground = std::move(func);
}

void vkr::Renderer::addDrawable(SceneObject const& drawableObject)
{
    Drawable const* drawable = drawableObject.getDrawable();

    if (!drawable)
        throw std::invalid_argument("drawableObject");

	auto const& material = drawable->getMaterial();

    vko::DescriptorSetConfiguration config;
    config.hasTexture = material.getTexture() != nullptr;
    config.hasNormalMap = material.getNormalMap() != nullptr;

	m_drawableInstances.emplace_back(getApp(), *drawable, drawableObject.getTransform(), sizeof(ObjectUniformBuffer), sizeof(MaterialUniformBuffer), sizeof(FrameUniformBuffer), m_swapchain->getImages().size());
}

void vkr::Renderer::clearObjects()
{
    m_drawableInstances.clear();
}

void vkr::Renderer::onFramebufferResized()
{
    m_framebufferResized = true;
}

void vkr::Renderer::draw()
{
    if (!m_activeCameraObject)
        throw std::runtime_error("No camera is set");

    FrameResources& frameResources = m_frameResources[m_nextFrameResourcesIndex];

    uint32_t imageIndex;
    VkResult aquireImageResult = vkAcquireNextImageKHR(getDevice().getHandle(), m_swapchain->getHandle(), std::numeric_limits<uint64_t>::max(), frameResources.imageAvailableSemaphore.getHandle(), VK_NULL_HANDLE, &imageIndex);

    if (aquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }

    if (aquireImageResult != VK_SUCCESS && aquireImageResult != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swapchain image!");

    m_fenceTimer.start();
    frameResources.inFlightFence.wait();
    m_lastFenceTime = m_fenceTimer.getTime();
    m_cumulativeFenceTime += m_lastFenceTime;
    
    frameResources.inFlightFence.reset();

    recordCommandBuffer(imageIndex, frameResources);
    frameResources.commandBuffer.submit(getDevice().getGraphicsQueue(), &frameResources.renderFinishedSemaphore, &frameResources.imageAvailableSemaphore, &frameResources.inFlightFence);

    std::array waitSemaphores{ frameResources.renderFinishedSemaphore.getHandle() };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    VkSwapchainKHR swapchains[] = { m_swapchain->getHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    VkResult presentResult = vkQueuePresentKHR(getDevice().getPresentQueue().getHandle(), &presentInfo); // TODO move to the object
    if (presentResult != VK_SUCCESS && presentResult != VK_ERROR_OUT_OF_DATE_KHR)
        throw std::runtime_error("vkQueuePresentKHR failed");

    if (aquireImageResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        recreateSwapchain();
    }

    m_nextFrameResourcesIndex = (m_nextFrameResourcesIndex + 1) % FRAME_RESOURCE_COUNT;
}

float vkr::Renderer::getAspect() const
{
    VkExtent2D extent = m_swapchain->getExtent();

    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
}

void vkr::Renderer::createSwapchain()
{
    m_pipelines.clear();

    PhysicalDeviceSurfaceParameters const& parameters = getPhysicalDeviceSurfaceParameters();

    vko::Swapchain::Config config;
    config.surfaceFormat = chooseSwapSurfaceFormat(parameters.getFormats());
    config.presentMode = chooseSwapPresentMode(parameters.getPresentModes());
    config.extent = chooseSwapExtent(getSurface(), parameters.getCapabilities());

    const uint32_t minImageCount = parameters.getCapabilities().minImageCount;
    const uint32_t maxImageCount = parameters.getCapabilities().maxImageCount;
    config.minImageCount = minImageCount + 1;
    if (maxImageCount > 0)
        config.minImageCount = std::min(config.minImageCount, maxImageCount);

    config.preTransform = parameters.getCapabilities().currentTransform;

    vkr::QueueFamilyIndices const& indices = parameters.getQueueFamilyIndices();

    m_swapchain = std::make_unique<vko::Swapchain>(getDevice(), getSurface(), indices.getGraphicsQueueFamily(), indices.getPresentQueueFamily(), std::move(config));

    VkFormat colorFormat = m_swapchain->getSurfaceFormat().format;
    VkFormat depthFormat = findDepthFormat(getPhysicalDevice());
    m_renderPass = std::make_unique<vko::RenderPass>(getDevice(), colorFormat, depthFormat);

    auto const& images = m_swapchain->getImages();
    m_swapchainImageViews.reserve(images.size());
    for (auto const& image : images)
        m_swapchainImageViews.push_back(std::make_unique<vko::ImageView>(image.createImageView(VK_IMAGE_ASPECT_COLOR_BIT)));

    // TODO move depth resources to the swapchain?
    VkExtent2D swapchainExtent = m_swapchain->getExtent();
    vkr::utils::createImage(getApp(), swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    m_depthImageView = std::make_unique<vko::ImageView>(m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT));

    m_swapchainFramebuffers.reserve(m_swapchainImageViews.size());
    for (std::unique_ptr<vko::ImageView> const& colorImageView : m_swapchainImageViews)
        m_swapchainFramebuffers.push_back(std::make_unique<vko::Framebuffer>(getDevice(), *colorImageView, *m_depthImageView, *m_renderPass, m_swapchain->getExtent()));

    onSwapchainCreated();
}

void vkr::Renderer::recreateSwapchain()
{
    if (m_waitUntilWindowInForeground)
        m_waitUntilWindowInForeground();

    getDevice().waitIdle();

    destroySwapchain();
    createSwapchain();
}

void vkr::Renderer::updateCameraAspect()
{
    if (!m_activeCameraObject)
        return;

	if (auto const& camera = m_activeCameraObject->getCamera())
		camera->setAspect(getAspect());
}

vkr::Renderer::UniformResources const& vkr::Renderer::getUniformResources(std::vector<vko::DescriptorSetConfiguration> const& configs)
{
    auto it = m_uniformResources.find(configs);

    if (it == m_uniformResources.end())
    {
        UniformResources resources;

        std::vector<VkDescriptorSetLayout> layouts;

        for (vko::DescriptorSetConfiguration const& config : configs)
        {
            auto layout = std::make_unique<vko::DescriptorSetLayout>(getDevice(), config);
            layouts.push_back(layout->getHandle());
            resources.descriptorSetLayouts.push_back(std::move(layout));
        }

        std::vector<VkPushConstantRange> ranges = {
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(VertexPushConstants),
            }
        };
        
        resources.pipelineLayout = std::make_unique<vko::PipelineLayout>(getDevice(), std::move(layouts), std::move(ranges));

        auto res = m_uniformResources.emplace(configs, std::move(resources));
        it = res.first;
    }

    return it->second;
}

void vkr::Renderer::onSwapchainCreated()
{
    updateCameraAspect();

    {
        m_oneFrameBoxResources = {};

        m_oneFrameBoxResources.bufferWithMemory = createBufferAndMemory(getApp(), boxBuffer);
        m_oneFrameBoxResources.mesh = std::make_unique<vkr::Mesh>(getApp(), m_oneFrameBoxResources.bufferWithMemory->buffer(), createBoxVertexLayout(), vkr::Mesh::Metadata{ false, false, true, false });

        std::vector<VkDescriptorSetLayout> layouts = {};

        std::vector<VkPushConstantRange> ranges = {
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(OneTimeGeometryPushConstants),
            }
        };
        m_oneFrameBoxResources.pipelineLayout = std::make_unique<vko::PipelineLayout>(getDevice(), std::move(layouts), std::move(ranges));

        vkr::PipelineConfiguration configuration;
        configuration.pipelineLayout = m_oneFrameBoxResources.pipelineLayout.get();
        configuration.shaderKey = vkr::Shader::Key{}
            .addStage(vko::ShaderModuleType::Vertex, "data/shaders/packaged/debugdraw.vert/shaders/debugdraw.vert.spv")
            .addStage(vko::ShaderModuleType::Fragment, "data/shaders/packaged/debugdraw.frag/shaders/debugdraw.frag.spv"); // TODO use ShaderPackage
        configuration.vertexLayoutDescriptions = m_oneFrameBoxResources.mesh->getVertexLayout().getDescriptions();
        configuration.cullBackFaces = false;
        configuration.wireframe = true;
        m_oneFrameBoxResources.pipeline = createPipeline(configuration);
    }
}

void vkr::Renderer::recordCommandBuffer(std::size_t imageIndex, FrameResources& frameResources)
{
    for (vko::DescriptorPool& pool : frameResources.descriptorPools)
        pool.reset();

    frameResources.commandPool->reset();

    CommandBuffer const& commandBuffer = frameResources.commandBuffer;

    commandBuffer.begin(true);

    VkCommandBuffer handle = commandBuffer.getHandle();

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass->getHandle();
    renderPassBeginInfo.framebuffer = m_swapchainFramebuffers[imageIndex]->getHandle(); // CRASH: get correct framebuffer
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = m_swapchain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    Camera* camera = m_activeCameraObject->getCamera();
    Transform const& cameraTransform = m_activeCameraObject->getTransform();

    for (ObjectInstance const& instance : m_drawableInstances)
    {
        Transform const& transform = instance.getTransform();

        Drawable const& drawable = instance.getDrawable();

        Mesh const& mesh = drawable.getMesh();
        Material const& material = drawable.getMaterial();

        {
            ObjectUniformBuffer values;
            values.objectColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

            instance.copyToObjectUniformBuffer(imageIndex, &values, sizeof(values));
        }

        {
            MaterialUniformBuffer values;
            values.objectColor = material.getColor();

            instance.copyToMaterialUniformBuffer(imageIndex, &values, sizeof(values));
        }

        {
            FrameUniformBuffer values;
            values.projection = camera->getProjectionMatrix();
            values.lightPosition = cameraTransform.getViewMatrix() * glm::vec4(m_light->getTransform().getLocalPos(), 1.0f);
            values.lightColor = m_light->getColor() * m_light->getIntensity();

            instance.copyToFrameUniformBuffer(imageIndex, &values, sizeof(values));
        }

        vko::DescriptorSetConfiguration config;
        config.hasTexture = material.getTexture() != nullptr;
        config.hasNormalMap = material.getNormalMap() != nullptr;

        std::vector<vko::DescriptorSetConfiguration> configs = {
            std::move(config),
            vko::DescriptorSetConfiguration{ false, false },
            vko::DescriptorSetConfiguration{ false, false },
        };

        auto const& resources = getUniformResources(configs);

        // TODO don't create heavy configuration for each object instance
        vkr::PipelineConfiguration configuration;
        configuration.pipelineLayout = resources.pipelineLayout.get();
        configuration.shaderKey = material.getShaderKey();
        configuration.vertexLayoutDescriptions = mesh.getVertexLayout().getDescriptions();
        configuration.cullBackFaces = !material.isDoubleSided();

        auto it = m_pipelines.find(configuration);
        if (it == m_pipelines.end())
            it = m_pipelines.emplace(configuration, createPipeline(configuration)).first;

        vko::Pipeline const& pipeline = *it->second;

        pipeline.bind(handle);

        {
            VertexPushConstants constants;
            constants.modelView = cameraTransform.getViewMatrix() * transform.getMatrix();
            constants.normal = glm::transpose(glm::inverse(constants.modelView));

            vkCmdPushConstants(handle, resources.pipelineLayout->getHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushConstants), &constants);
        }

        {
            std::vector<vko::DescriptorPool>& descriptorPools = frameResources.descriptorPools;

            std::vector<VkDescriptorSetLayout> layouts;
            for (auto const& layout : resources.descriptorSetLayouts)
                layouts.push_back(layout->getHandle());

            std::optional<vko::DescriptorSets> descriptorSets;
            do
            {
                if (!descriptorPools.empty())
                    descriptorSets = descriptorPools.back().allocate(layouts);

                if (!descriptorSets)
                    descriptorPools.emplace_back(getDevice());
            }
            while (!descriptorSets);

            vkr::BufferWithMemory const& objectUniformBuffer = instance.getObjectBuffer();
            vkr::BufferWithMemory const& materialUniformBuffer = instance.getMaterialBuffer();
            vkr::BufferWithMemory const& frameUniformBuffer = instance.getFrameBuffer();

            vko::DescriptorSets::UpdateConfig config;
            config.buffers = {
                {
                    .set = 0,
                    .binding = 0,
                    .buffer = objectUniformBuffer.buffer().getHandle(),
                    .offset = 0,
                    .size = sizeof(ObjectUniformBuffer),
                },
                {
                    .set = 1,
                    .binding = 0,
                    .buffer = materialUniformBuffer.buffer().getHandle(),
                    .offset = 0,
                    .size = sizeof(MaterialUniformBuffer),
                },
                {
                    .set = 2,
                    .binding = 0,
                    .buffer = frameUniformBuffer.buffer().getHandle(),
                    .offset = 0,
                    .size = sizeof(FrameUniformBuffer),
                },
            };
            config.images = {
                {
                    .set = 0,
                    .binding = 1,
                    .imageView = material.getTexture()->getImageView().getHandle(),
                    .sampler = material.getTexture()->getSampler().getHandle(),
                },
                {
                    .set = 0,
                    .binding = 2,
                    .imageView = material.getNormalMap()->getImageView().getHandle(),
                    .sampler = material.getNormalMap()->getSampler().getHandle(),
                },
            };
            descriptorSets->update(config);

            std::array descriptorSetHandles = { descriptorSets->getHandle(0), descriptorSets->getHandle(1), descriptorSets->getHandle(2) };
            auto dynamicOffsets = instance.getBufferOffsets(imageIndex);

            vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, resources.pipelineLayout->getHandle(), 0, descriptorSetHandles.size(), descriptorSetHandles.data(), dynamicOffsets.size(), dynamicOffsets.data());
        }

        mesh.draw(handle);
    }

    // TODO find a better place
    {
        m_oneFrameBoxResources.pipeline->bind(handle);

        Camera* camera = m_activeCameraObject->getCamera();

        for (auto const& instance : m_oneFrameBoxInstances)
        {
            OneTimeGeometryPushConstants constants;
            constants.modelViewProjection = camera->getProjectionMatrix() * cameraTransform.getViewMatrix() * instance.model;

            vkCmdPushConstants(handle, m_oneFrameBoxResources.pipelineLayout->getHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(OneTimeGeometryPushConstants), &constants);
            m_oneFrameBoxResources.mesh->draw(handle);
        }

        m_oneFrameBoxInstances.clear();
    }

    // TODO abstract this away
    if (ImGui::GetCurrentContext())
        if (ImDrawData* drawData = ImGui::GetDrawData())
            ImGui_ImplVulkan_RenderDrawData(drawData, handle);

    vkCmdEndRenderPass(handle);

    commandBuffer.end();
}

std::unique_ptr<vko::Pipeline> vkr::Renderer::createPipeline(PipelineConfiguration const& configuration)
{
    vko::PipelineLayout const& layout = *configuration.pipelineLayout;
    vko::RenderPass const& renderPass = *m_renderPass;

    vkr::Shader shader{ getDevice(), configuration.shaderKey };

    vko::Pipeline::Config config;
    config.extent = m_swapchain->getExtent();
    config.bindingDescriptions = configuration.vertexLayoutDescriptions.getBindingDescriptions(); // TODO avoid copy
    config.attributeDescriptions = configuration.vertexLayoutDescriptions.getAttributeDescriptions(); // TODO avoid copy
    config.cullBackFaces = configuration.cullBackFaces;
    config.wireframe = configuration.wireframe;

    std::vector<vko::ShaderModule const*> shaderModules;
    for (vko::ShaderModule const& module : shader.getModules())
        shaderModules.push_back(&module);

    return std::make_unique<vko::Pipeline>(getDevice(), layout, renderPass, shaderModules, config);
}

void vkr::Renderer::destroySwapchain()
{
	m_swapchainFramebuffers.clear();
	m_depthImageView = nullptr;
	m_depthImage = nullptr;
	m_depthImageMemory = nullptr;
	m_swapchainImageViews.clear();
	m_renderPass = nullptr;
	m_swapchain = nullptr;
}

void vkr::Renderer::createSyncObjects()
{
    for (auto i = 0; i < FRAME_RESOURCE_COUNT; i++)
        m_frameResources.emplace_back(getApp());
}

vkr::Renderer::FrameResources::FrameResources(Application const& app)
    : imageAvailableSemaphore(app.getDevice())
    , renderFinishedSemaphore(app.getDevice())
    , inFlightFence(app.getDevice())
    , commandPool(std::make_unique<vko::CommandPool>(app.getDevice(), app.getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getGraphicsQueueFamily()))
    , commandBuffer(std::make_shared<vko::CommandBuffers>(commandPool->createCommandBuffers(1)), 0)
{
    app.setDebugName(imageAvailableSemaphore.getHandle(), "ImageAvailableSemaphore");
    app.setDebugName(renderFinishedSemaphore.getHandle(), "RenderFinishedSemaphore");
}
