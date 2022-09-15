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

#include "vkgfx/ResourceManager.h"

// TODO remove vkr references
#include "Application.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"

#include <vulkan/vulkan.h>

#include <array>

namespace
{
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

    m_swapchain = std::make_unique<vko::Swapchain>(m_application->getDevice(), m_application->getSurface(), indices.getGraphicsQueueFamily(), indices.getPresentQueueFamily(), std::move(config));

    VkFormat colorFormat = m_swapchain->getSurfaceFormat().format;
    VkFormat depthFormat = findDepthFormat(physicalDevice);
    m_renderPass = std::make_unique<vko::RenderPass>(m_application->getDevice(), colorFormat, depthFormat);

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
        m_swapchainFramebuffers.push_back(std::make_unique<vko::Framebuffer>(m_application->getDevice(), *colorImageView, *m_depthImageView, *m_renderPass, m_swapchain->getExtent()));

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

    m_resourceManager = std::make_unique<ResourceManager>(device, physicalDevice, m_application->getShortLivedCommandPool(), device.getGraphicsQueue(), *m_renderPass, swapchainExtent.width, swapchainExtent.height);
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

void vkgfx::Renderer::recordCommandBuffer(std::size_t imageIndex, RendererFrameResources& frameResources)
{
    for (vko::DescriptorPool& pool : frameResources.descriptorPools)
        pool.reset();

    frameResources.commandPool.reset();

    vko::CommandBuffers const& commandBuffers = frameResources.commandBuffers;

    commandBuffers.begin(0, true);

    VkCommandBuffer handle = commandBuffers.getHandle(0);

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

//     Camera* camera = m_activeCameraObject->getCamera();
//     Transform const& cameraTransform = m_activeCameraObject->getTransform();
// 
//     for (ObjectInstance const& instance : m_drawableInstances)
//     {
//         Transform const& transform = instance.getTransform();
// 
//         Drawable const& drawable = instance.getDrawable();
// 
//         Mesh const& mesh = drawable.getMesh();
//         Material const& material = drawable.getMaterial();
// 
//         {
//             ObjectUniformBuffer values;
//             values.objectColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
// 
//             instance.copyToObjectUniformBuffer(imageIndex, &values, sizeof(values));
//         }
// 
//         {
//             MaterialUniformBuffer values;
//             values.objectColor = material.getColor();
// 
//             instance.copyToMaterialUniformBuffer(imageIndex, &values, sizeof(values));
//         }
// 
//         {
//             FrameUniformBuffer values;
//             values.projection = camera->getProjectionMatrix();
//             values.lightPosition = cameraTransform.getViewMatrix() * glm::vec4(m_light->getTransform().getLocalPos(), 1.0f);
//             values.lightColor = m_light->getColor() * m_light->getIntensity();
// 
//             instance.copyToFrameUniformBuffer(imageIndex, &values, sizeof(values));
//         }
// 
//         vko::DescriptorSetConfiguration config;
//         config.hasTexture = material.getTexture() != nullptr;
//         config.hasNormalMap = material.getNormalMap() != nullptr;
// 
//         std::vector<vko::DescriptorSetConfiguration> configs = {
//             std::move(config),
//             vko::DescriptorSetConfiguration{ false, false },
//             vko::DescriptorSetConfiguration{ false, false },
//         };
// 
//         auto const& resources = getUniformResources(configs);
// 
//         // TODO don't create heavy configuration for each object instance
//         vkr::PipelineConfiguration configuration;
//         configuration.pipelineLayout = resources.pipelineLayout.get();
//         configuration.shaderKey = material.getShaderKey();
//         configuration.vertexLayoutDescriptions = mesh.getVertexLayout().getDescriptions();
//         configuration.cullBackFaces = !material.isDoubleSided();
// 
//         auto it = m_pipelines.find(configuration);
//         if (it == m_pipelines.end())
//             it = m_pipelines.emplace(configuration, createPipeline(configuration)).first;
// 
//         vko::Pipeline const& pipeline = *it->second;
// 
//         pipeline.bind(handle);
// 
//         {
//             VertexPushConstants constants;
//             constants.modelView = cameraTransform.getViewMatrix() * transform.getMatrix();
//             constants.normal = glm::transpose(glm::inverse(constants.modelView));
// 
//             vkCmdPushConstants(handle, resources.pipelineLayout->getHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushConstants), &constants);
//         }
// 
//         {
//             std::vector<vko::DescriptorPool>& descriptorPools = frameResources.descriptorPools;
// 
//             std::vector<VkDescriptorSetLayout> layouts;
//             for (auto const& layout : resources.descriptorSetLayouts)
//                 layouts.push_back(layout->getHandle());
// 
//             std::optional<vko::DescriptorSets> descriptorSets;
//             do
//             {
//                 if (!descriptorPools.empty())
//                     descriptorSets = descriptorPools.back().allocate(layouts);
// 
//                 if (!descriptorSets)
//                     descriptorPools.emplace_back(getDevice());
//             } while (!descriptorSets);
// 
//             vkr::BufferWithMemory const& objectUniformBuffer = instance.getObjectBuffer();
//             vkr::BufferWithMemory const& materialUniformBuffer = instance.getMaterialBuffer();
//             vkr::BufferWithMemory const& frameUniformBuffer = instance.getFrameBuffer();
// 
//             vko::DescriptorSets::UpdateConfig config;
//             config.buffers = {
//                 {
//                     .set = 0,
//                     .binding = 0,
//                     .buffer = objectUniformBuffer.buffer().getHandle(),
//                     .offset = 0,
//                     .size = sizeof(ObjectUniformBuffer),
//                 },
//                 {
//                     .set = 1,
//                     .binding = 0,
//                     .buffer = materialUniformBuffer.buffer().getHandle(),
//                     .offset = 0,
//                     .size = sizeof(MaterialUniformBuffer),
//                 },
//                 {
//                     .set = 2,
//                     .binding = 0,
//                     .buffer = frameUniformBuffer.buffer().getHandle(),
//                     .offset = 0,
//                     .size = sizeof(FrameUniformBuffer),
//                 },
//             };
//             config.images = {
//                 {
//                     .set = 0,
//                     .binding = 1,
//                     .imageView = material.getTexture()->getImageView().getHandle(),
//                     .sampler = material.getTexture()->getSampler().getHandle(),
//                 },
//                 {
//                     .set = 0,
//                     .binding = 2,
//                     .imageView = material.getNormalMap()->getImageView().getHandle(),
//                     .sampler = material.getNormalMap()->getSampler().getHandle(),
//                 },
//             };
//             descriptorSets->update(config);
// 
//             std::array descriptorSetHandles = { descriptorSets->getHandle(0), descriptorSets->getHandle(1), descriptorSets->getHandle(2) };
//             auto dynamicOffsets = instance.getBufferOffsets(imageIndex);
// 
//             vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, resources.pipelineLayout->getHandle(), 0, descriptorSetHandles.size(), descriptorSetHandles.data(), dynamicOffsets.size(), dynamicOffsets.data());
//         }
// 
//         mesh.draw(handle);
//     }

//     // TODO find a better place
//     {
//         m_oneFrameBoxResources.pipeline->bind(handle);
// 
//         Camera* camera = m_activeCameraObject->getCamera();
// 
//         for (auto const& instance : m_oneFrameBoxInstances)
//         {
//             OneTimeGeometryPushConstants constants;
//             constants.modelViewProjection = camera->getProjectionMatrix() * cameraTransform.getViewMatrix() * instance.model;
// 
//             vkCmdPushConstants(handle, m_oneFrameBoxResources.pipelineLayout->getHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(OneTimeGeometryPushConstants), &constants);
//             m_oneFrameBoxResources.mesh->draw(handle);
//         }
// 
//         m_oneFrameBoxInstances.clear();
//     }

//     // TODO abstract this away
//     if (ImGui::GetCurrentContext())
//         if (ImDrawData* drawData = ImGui::GetDrawData())
//             ImGui_ImplVulkan_RenderDrawData(drawData, handle);

    vkCmdEndRenderPass(handle);

    commandBuffers.end(0);
}
