#include "Renderer.h"

#include "wrapper/Swapchain.h"
#include "wrapper/Framebuffer.h"
#include "wrapper/RenderPass.h"
#include "wrapper/Image.h"
#include "wrapper/ImageView.h"
#include "wrapper/DeviceMemory.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Surface.h"
#include "wrapper/Semaphore.h"
#include "wrapper/Fence.h"
#include "wrapper/DescriptorPool.h"
#include "wrapper/CommandPool.h"
#include "wrapper/CommandBuffers.h"
#include "wrapper/Device.h"
#include "wrapper/Queue.h"
#include "wrapper/Pipeline.h"
#include "wrapper/Buffer.h"

#include "ResourceManager.h"
#include "TestObject.h"
#include "Mesh.h"
#include "Buffer.h"
#include "Material.h"
#include "Texture.h"
#include "Image.h"
#include "Sampler.h"

// TODO remove vkr references
#include "Application.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "glm.h"

#include <vulkan/vulkan.h>

#include <array>

namespace
{
    struct CameraData
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec3 lightPosition;
        glm::vec3 lightColor;
    };

    int const FRAME_RESOURCE_COUNT = 3;

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

    VkExtent2D chooseSwapchainExtent(vko::Surface const& surface, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        auto width = static_cast<uint32_t>(surface.getWidth());
        auto height = static_cast<uint32_t>(surface.getHeight());

        width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
        height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));

        return { width, height };
    }

    VkIndexType vulkanizeIndexType(vkgfx::IndexType type)
    {
        switch (type)
        {
        case vkgfx::IndexType::UnsignedByte:
            return VK_INDEX_TYPE_UINT8_EXT;
        case vkgfx::IndexType::UnsignedShort:
            return VK_INDEX_TYPE_UINT16;
        case vkgfx::IndexType::UnsignedInt:
            return VK_INDEX_TYPE_UINT32;
        }

        throw std::invalid_argument("type");
    }
}

namespace vkgfx
{
    struct RendererFrameResources
    {
        vko::Semaphore imageAvailableSemaphore;
        vko::Semaphore renderFinishedSemaphore;
        vko::Fence inFlightFence;

        vko::CommandPool commandPool;
        vko::CommandBuffers commandBuffers;

        std::vector<vko::DescriptorPool> descriptorPools;
    };
}

vkgfx::Renderer::Renderer(std::string const& name, bool enableValidationLayers, vko::Window const& window, std::function<void(vko::DebugMessage)> onDebugMessage) : m_window(window)
{
    m_application = std::make_unique<vkr::Application>(name, enableValidationLayers, false, window, std::move(onDebugMessage));

    vko::Device const& device = m_application->getDevice();
    vko::PhysicalDevice const& physicalDevice = m_application->getPhysicalDevice();

    vkr::PhysicalDeviceSurfaceParameters const& parameters = m_application->getPhysicalDeviceSurfaceParameters();

    vko::Swapchain::Config config;
    config.surfaceFormat = chooseSwapSurfaceFormat(parameters.getFormats());
    config.presentMode = chooseSwapPresentMode(parameters.getPresentModes());
    config.extent = chooseSwapchainExtent(m_application->getSurface(), parameters.getCapabilities());

    const uint32_t minImageCount = parameters.getCapabilities().minImageCount;
    const uint32_t maxImageCount = parameters.getCapabilities().maxImageCount;
    config.minImageCount = minImageCount + 1;
    if (maxImageCount > 0)
        config.minImageCount = std::min(config.minImageCount, maxImageCount);

    config.preTransform = parameters.getCapabilities().currentTransform;

    vkr::QueueFamilyIndices const& indices = parameters.getQueueFamilyIndices();

    m_swapchain = std::make_unique<vko::Swapchain>(device, m_application->getSurface(), indices.getGraphicsQueueFamily(), indices.getPresentQueueFamily(), std::move(config));

    VkFormat colorFormat = m_swapchain->getSurfaceFormat().format;
    VkFormat depthFormat = findDepthFormat(physicalDevice);
    m_renderPass = std::make_unique<vko::RenderPass>(device, colorFormat, depthFormat);

    auto const& images = m_swapchain->getImages();
    m_swapchainImageViews.reserve(images.size());
    for (auto const& image : images)
        m_swapchainImageViews.push_back(std::make_unique<vko::ImageView>(image.createImageView(VK_IMAGE_ASPECT_COLOR_BIT)));

    // TODO move depth resources to the swapchain?
    VkExtent2D swapchainExtent = m_swapchain->getExtent();
    vkr::utils::createImage(*m_application, swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    m_depthImageView = std::make_unique<vko::ImageView>(m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT));

    m_swapchainFramebuffers.reserve(m_swapchainImageViews.size());
    for (std::unique_ptr<vko::ImageView> const& colorImageView : m_swapchainImageViews)
        m_swapchainFramebuffers.push_back(std::make_unique<vko::Framebuffer>(device, *colorImageView, *m_depthImageView, *m_renderPass, m_swapchain->getExtent()));

    vko::QueueFamily const& graphicsQueueFamily = m_application->getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getGraphicsQueueFamily();

    for (auto i = 0; i < FRAME_RESOURCE_COUNT; i++)
    {
        RendererFrameResources resources
        {
            .imageAvailableSemaphore{device},
            .renderFinishedSemaphore{device},
            .inFlightFence{device},
            .commandPool{device, graphicsQueueFamily},
            .commandBuffers{resources.commandPool.allocate(1)},
        };
        m_frameResources.push_back(std::move(resources));
    }

    m_resourceManager = std::make_unique<ResourceManager>(device, physicalDevice, m_application->getShortLivedCommandPool(), device.getGraphicsQueue(), *m_renderPass, FRAME_RESOURCE_COUNT, swapchainExtent.width, swapchainExtent.height);

    createCameraResources();

    // TODO move to utils
    auto quatFromEuler = [](glm::vec3 const& eulerDegrees)
    {
        return glm::quat{ glm::radians(eulerDegrees) };
    };

    // TODO remove hardcoded values
    m_cameraTransform = {
        .position = {5.0f, 3.5f, 0.0f},
        .rotation = quatFromEuler({-15.0f, 90.0f, 0.0f}),
    };

    m_cameraParameters = {
        .fov = 45.0f,
        .nearZ = 0.1f,
        .farZ = 10000.0f,
    };

    m_lightParameters = {
        .position = {0.0, 2.0f, 0.0f},
        .color = {1.0, 0.0f, 0.0f},
        .intensity = 30.0f,
    };
}

vkgfx::Renderer::~Renderer()
{
    m_application->getDevice().waitIdle();

    m_swapchainFramebuffers.clear();
    m_depthImageView = nullptr;
    m_depthImage = nullptr;
    m_depthImageMemory = nullptr;
    m_swapchainImageViews.clear();
    m_renderPass = nullptr;
    m_swapchain = nullptr;
}

void vkgfx::Renderer::addTestObject(TestObject object)
{
    m_testObjects.push_back(std::move(object));
}

void vkgfx::Renderer::setCameraTransform(TestCameraTransform transform)
{
    m_cameraTransform = std::move(transform);
}

void vkgfx::Renderer::draw()
{
    RendererFrameResources& frameResources = m_frameResources[m_nextFrameResourcesIndex];

    vko::Device const& device = m_application->getDevice();

    uint32_t imageIndex;
    VkResult aquireImageResult = vkAcquireNextImageKHR(device.getHandle(), m_swapchain->getHandle(), std::numeric_limits<uint64_t>::max(), frameResources.imageAvailableSemaphore.getHandle(), VK_NULL_HANDLE, &imageIndex);

//     if (aquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
//     {
//         recreateSwapchain();
//         return;
//     }

    if (aquireImageResult != VK_SUCCESS && aquireImageResult != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swapchain image!");

//     m_fenceTimer.start();
    frameResources.inFlightFence.wait();
//     m_lastFenceTime = m_fenceTimer.getTime();
//     m_cumulativeFenceTime += m_lastFenceTime;

    frameResources.inFlightFence.reset();

    // TODO don't update camera buffer every frame
    // TODO update camera buffer directly without staging buffer
    updateCameraBuffer();
    m_resourceManager->transferDynamicBuffersFromStaging(m_nextFrameResourcesIndex);

    recordCommandBuffer(imageIndex, frameResources);
    frameResources.commandBuffers.submit(0, device.getGraphicsQueue(), &frameResources.renderFinishedSemaphore, &frameResources.imageAvailableSemaphore, &frameResources.inFlightFence);

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

    VkResult presentResult = vkQueuePresentKHR(device.getPresentQueue().getHandle(), &presentInfo); // TODO move to the object
    if (presentResult != VK_SUCCESS && presentResult != VK_ERROR_OUT_OF_DATE_KHR)
        throw std::runtime_error("vkQueuePresentKHR failed");

//     if (aquireImageResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
//     {
//         m_framebufferResized = false;
//         recreateSwapchain();
//     }

    m_nextFrameResourcesIndex = (m_nextFrameResourcesIndex + 1) % FRAME_RESOURCE_COUNT;
}

void vkgfx::Renderer::createCameraResources()
{
    BufferMetadata metadata{
        .usage = vkgfx::BufferUsage::UniformBuffer,
        .location = vkgfx::BufferLocation::HostVisible,
        .isMutable = true,
    };
    m_cameraBuffer = m_resourceManager->createBuffer(sizeof(CameraData), std::move(metadata));
}

void vkgfx::Renderer::recordCommandBuffer(std::size_t imageIndex, RendererFrameResources& frameResources)
{
    for (vko::DescriptorPool& pool : frameResources.descriptorPools)
        pool.reset();

    frameResources.commandPool.reset();

    vko::CommandBuffers const& commandBuffers = frameResources.commandBuffers;

    commandBuffers.begin(0, true);

    VkCommandBuffer commandBuffer = commandBuffers.getHandle(0);

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

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (TestObject const& object : m_testObjects)
    {
        vko::Pipeline const& pipeline = m_resourceManager->getPipeline(object.pipeline);
        pipeline.bind(commandBuffer);

        if (!object.pushConstants.empty())
            vkCmdPushConstants(commandBuffer, pipeline.getPipelineLayoutHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, object.pushConstants.size(), object.pushConstants.data()); // TODO configure shader stage

        {
            std::vector<vko::DescriptorPool>& descriptorPools = frameResources.descriptorPools;

            std::optional<vko::DescriptorSets> descriptorSets;
            do
            {
                if (!descriptorPools.empty())
                    descriptorSets = descriptorPools.back().allocate(pipeline.getDescriptorSetLayouts());

                if (!descriptorSets)
                    descriptorPools.emplace_back(m_application->getDevice());
            } while (!descriptorSets);

            Material const& material = m_resourceManager->getMaterial(object.material);
            Buffer const& materialUniformBuffer = m_resourceManager->getBuffer(material.uniformBuffer);
            Buffer const& objectUniformBuffer = m_resourceManager->getBuffer(object.uniformBuffer);
            Buffer const& frameUniformBuffer = m_resourceManager->getBuffer(m_cameraBuffer);

            Texture const& albedoTexture = m_resourceManager->getTexture(material.albedo);
            Image const& albedoImage = m_resourceManager->getImage(albedoTexture.image);
            Sampler const& albedoSampler = m_resourceManager->getSampler(albedoTexture.sampler);
            Texture const& normalMapTexture = m_resourceManager->getTexture(material.normalMap);
            Image const& normalMapImage = m_resourceManager->getImage(normalMapTexture.image);
            Sampler const& normalMapSampler = m_resourceManager->getSampler(normalMapTexture.sampler);

            vko::DescriptorSets::UpdateConfig config;
            config.buffers = {
                {
                    .set = 0,
                    .binding = 0,
                    .buffer = objectUniformBuffer.buffer.getHandle(),
                    .offset = 0,
                    .size = objectUniformBuffer.size,
                },
                {
                    .set = 1,
                    .binding = 0,
                    .buffer = materialUniformBuffer.buffer.getHandle(),
                    .offset = 0,
                    .size = materialUniformBuffer.size,
                },
                {
                    .set = 2,
                    .binding = 0,
                    .buffer = frameUniformBuffer.buffer.getHandle(),
                    .offset = 0,
                    .size = frameUniformBuffer.size,
                },
            };
            config.images = {
                {
                    .set = 0,
                    .binding = 1,
                    .imageView = albedoImage.imageView.getHandle(),
                    .sampler = albedoSampler.sampler.getHandle(),
                },
                {
                    .set = 0,
                    .binding = 2,
                    .imageView = normalMapImage.imageView.getHandle(),
                    .sampler = normalMapSampler.sampler.getHandle(),
                },
            };
            descriptorSets->update(config);

            std::array descriptorSetHandles = { descriptorSets->getHandle(0), descriptorSets->getHandle(1), descriptorSets->getHandle(2) };
            std::array<uint32_t, 3> dynamicOffsets = {
                static_cast<uint32_t>(objectUniformBuffer.getDynamicOffset(m_nextFrameResourcesIndex)),
                static_cast<uint32_t>(materialUniformBuffer.getDynamicOffset(m_nextFrameResourcesIndex)),
                static_cast<uint32_t>(frameUniformBuffer.getDynamicOffset(m_nextFrameResourcesIndex)),
            };

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayoutHandle(), 0, descriptorSetHandles.size(), descriptorSetHandles.data(), dynamicOffsets.size(), dynamicOffsets.data());
        }

        Mesh const& mesh = m_resourceManager->getMesh(object.mesh);
        
        std::vector<VkBuffer> vertexBuffers;
        std::vector<VkDeviceSize> vertexBuffersOffsets;
        for (BufferWithOffset const& bufferWithOffset : mesh.vertexBuffers)
        {
            Buffer const& vertexBuffer = m_resourceManager->getBuffer(bufferWithOffset.buffer);
            vertexBuffers.push_back(vertexBuffer.buffer.getHandle());
            vertexBuffersOffsets.push_back(bufferWithOffset.offset);
        }

        Buffer const& indexBuffer = m_resourceManager->getBuffer(mesh.indexBuffer.buffer);

        vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(vertexBuffersOffsets.size()), vertexBuffers.data(), vertexBuffersOffsets.data());
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer.getHandle(), mesh.indexBuffer.offset, vulkanizeIndexType(mesh.indexType));

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indexCount), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);

    commandBuffers.end(0);
}

void vkgfx::Renderer::updateCameraBuffer()
{
    VkExtent2D extent = m_swapchain->getExtent();
    auto aspectRatio = 1.0f * extent.width / extent.height;

    CameraData data{
        .view = glm::inverse(glm::translate(glm::mat4(1.0f), m_cameraTransform.position) * glm::toMat4(m_cameraTransform.rotation)),
        .projection = glm::perspective(glm::radians(m_cameraParameters.fov), aspectRatio, m_cameraParameters.nearZ, m_cameraParameters.farZ),
        .lightPosition = data.view * glm::vec4(m_lightParameters.position, 1.0f),
        .lightColor = m_lightParameters.intensity * m_lightParameters.color,
    };
    data.projection[1][1] *= -1;

    m_resourceManager->uploadDynamicBufferToStaging(m_cameraBuffer, &data, sizeof(data));
}
