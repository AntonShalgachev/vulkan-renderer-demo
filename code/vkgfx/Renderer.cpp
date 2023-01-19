#include "Renderer.h"

#include "vko/Swapchain.h"
#include "vko/Framebuffer.h"
#include "vko/RenderPass.h"
#include "vko/Image.h"
#include "vko/ImageView.h"
#include "vko/DeviceMemory.h"
#include "vko/PhysicalDevice.h"
#include "vko/Surface.h"
#include "vko/Semaphore.h"
#include "vko/Fence.h"
#include "vko/DescriptorPool.h"
#include "vko/CommandPool.h"
#include "vko/CommandBuffers.h"
#include "vko/Device.h"
#include "vko/Queue.h"
#include "vko/Pipeline.h"
#include "vko/Buffer.h"
#include "vko/Window.h"
#include "vko/DescriptorSetLayout.h"
#include "vko/Instance.h"
#include "vko/Sampler.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"

#include "vkgfx/Context.h"
#include "vkgfx/ResourceManager.h"
#include "vkgfx/TestObject.h"
#include "vkgfx/Mesh.h"
#include "vkgfx/Buffer.h"
#include "vkgfx/Material.h"
#include "vkgfx/Texture.h"
#include "vkgfx/Image.h"
#include "vkgfx/PipelineKey.h"

#include "glm.h"

#include "nstl/array.h"
#include "nstl/algorithm.h"
#include "nstl/sprintf.h"
#include "nstl/span.h"

#include "memory/tracking.h"

#include <vulkan/vulkan.h>

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

    VkFormat findSupportedFormat(vko::PhysicalDevice const& physicalDevice, nstl::span<VkFormat const> candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

        assert(false);
        return VkFormat{};
    }

    VkFormat findDepthFormat(vko::PhysicalDevice const& physicalDevice)
    {
        VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        return findSupportedFormat(physicalDevice, candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(nstl::span<VkSurfaceFormatKHR const> availableFormats)
    {
        if (availableFormats.empty())
            return { VK_FORMAT_UNDEFINED , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(nstl::span<VkPresentModeKHR const> availablePresentModes)
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

        width = nstl::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        height = nstl::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

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

        assert(false);
        return VK_INDEX_TYPE_UINT32;
    }

    struct DescriptorSetUpdateConfig
    {
        struct SampledImage
        {
            std::size_t binding = 0;
            VkImageView imageView = VK_NULL_HANDLE;
            VkSampler sampler = VK_NULL_HANDLE;
        };

        struct Buffer
        {
            std::size_t binding = 0;
            VkBuffer buffer = VK_NULL_HANDLE;
            std::size_t offset = 0;
            std::size_t size = 0;
        };

        nstl::vector<SampledImage> images;
        nstl::vector<Buffer> buffers;

        void clear()
        {
            images.clear();
            buffers.clear();
        }
    };
}

namespace vkgfx
{
    struct RendererData
    {
        VkSurfaceFormatKHR m_surfaceFormat{};
        VkFormat m_depthFormat{};

        vko::DescriptorPool frameDescriptorPool;
        VkDescriptorSetLayout frameDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet frameDescriptorSet = VK_NULL_HANDLE;
    };

    struct RendererFrameResources
    {
        vko::Semaphore imageAvailableSemaphore;
        vko::Semaphore renderFinishedSemaphore;
        vko::Fence inFlightFence;

        vko::CommandPool commandPool;
        vko::CommandBuffers commandBuffers;

        nstl::vector<vko::DescriptorPool> descriptorPools;
    };

    struct RendererCache
    {
        nstl::vector<VkBuffer> vertexBuffers;
        nstl::vector<VkDeviceSize> vertexBuffersOffsets;
        nstl::vector<VkDescriptorSet> descriptorSets;

        DescriptorSetUpdateConfig config1;
        DescriptorSetUpdateConfig config2;
        nstl::vector<uint32_t> dynamicBufferOffsets12;

        nstl::vector<VkDescriptorBufferInfo> bufferInfos;
        nstl::vector<VkDescriptorImageInfo> imageInfos;
        nstl::vector<VkWriteDescriptorSet> descriptorWrites;
    };
}

namespace
{
    void updateDescriptorSet(VkDevice device, VkDescriptorSet set, DescriptorSetUpdateConfig const& config, vkgfx::RendererCache& cache)
    {
        static auto scopeId = memory::tracking::create_scope_id("Rendering/Draw/UpdateDescriptorSet");
        MEMORY_TRACKING_SCOPE(scopeId);

        cache.bufferInfos.clear();
        cache.bufferInfos.reserve(config.buffers.size());

        cache.imageInfos.clear();
        cache.imageInfos.reserve(config.images.size());

        cache.descriptorWrites.clear();
        cache.descriptorWrites.reserve(cache.bufferInfos.capacity() + cache.imageInfos.capacity());

        for (DescriptorSetUpdateConfig::Buffer const& buffer : config.buffers)
        {
            VkWriteDescriptorSet& descriptorWrite = cache.descriptorWrites.emplace_back();

            assert(cache.bufferInfos.size() < cache.bufferInfos.capacity());
            VkDescriptorBufferInfo& bufferInfo = cache.bufferInfos.emplace_back();
            bufferInfo.buffer = buffer.buffer;
            bufferInfo.offset = buffer.offset;
            bufferInfo.range = buffer.size;

            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = set;
            descriptorWrite.dstBinding = buffer.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
        }

        for (DescriptorSetUpdateConfig::SampledImage const& image : config.images)
        {
            assert(cache.descriptorWrites.size() < cache.descriptorWrites.capacity());
            VkWriteDescriptorSet& descriptorWrite = cache.descriptorWrites.emplace_back();

            assert(cache.imageInfos.size() < cache.imageInfos.capacity());
            VkDescriptorImageInfo& imageInfo = cache.imageInfos.emplace_back();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = image.imageView;
            imageInfo.sampler = image.sampler;

            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = set;
            descriptorWrite.dstBinding = image.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(cache.descriptorWrites.size()), cache.descriptorWrites.data(), 0, nullptr);
    }
}

vkgfx::Renderer::Renderer(char const* name, bool enableValidationLayers, vko::Window& window, nstl::function<void(vko::DebugMessage)> onDebugMessage) : m_window(window)
{
    m_cache = nstl::make_unique<RendererCache>();

    m_window.addFramebufferResizeCallback([this](int, int) { onWindowResized(); });

    m_context = nstl::make_unique<Context>(name, enableValidationLayers, false, window, nstl::move(onDebugMessage));

    vko::Instance const& instance = m_context->getInstance();
    vko::Device const& device = m_context->getDevice();
    vko::PhysicalDevice const& physicalDevice = m_context->getPhysicalDevice();

    instance.setDebugName(device.getHandle(), device.getHandle(), "Device");
    instance.setDebugName(device.getHandle(), device.getGraphicsQueue().getHandle(), "Graphics");
    instance.setDebugName(device.getHandle(), device.getPresentQueue().getHandle(), "Present");

    vko::PhysicalDeviceSurfaceParameters const& parameters = m_context->getPhysicalDeviceSurfaceParameters();
    vko::QueueFamily const& graphicsQueueFamily = *parameters.graphicsQueueFamily;
    nstl::span<VkSurfaceFormatKHR const> formats = parameters.formats;

    m_data = nstl::make_unique<RendererData>(RendererData{
        .frameDescriptorPool{device},
    });

    m_data->m_surfaceFormat = chooseSwapSurfaceFormat(formats);
    m_data->m_depthFormat = findDepthFormat(physicalDevice);

    {
        m_renderPass = nstl::make_unique<vko::RenderPass>(device, m_data->m_surfaceFormat.format, m_data->m_depthFormat);
        instance.setDebugName(device.getHandle(), m_renderPass->getHandle(), "Main");
    }

    createSwapchain();

    for (auto i = 0; i < FRAME_RESOURCE_COUNT; i++)
    {
        vko::CommandPool commandPool{ device, graphicsQueueFamily };
        vko::CommandBuffers commandBuffers{ commandPool.allocate(1) };

        RendererFrameResources resources
        {
            .imageAvailableSemaphore{device},
            .renderFinishedSemaphore{device},
            .inFlightFence{device},
            .commandPool{std::move(commandPool)},
            .commandBuffers{std::move(commandBuffers)},
        };

        instance.setDebugName(device.getHandle(), resources.imageAvailableSemaphore.getHandle(), nstl::sprintf("Image available %d", i));
        instance.setDebugName(device.getHandle(), resources.renderFinishedSemaphore.getHandle(), nstl::sprintf("Render finished %d", i));
        instance.setDebugName(device.getHandle(), resources.inFlightFence.getHandle(), nstl::sprintf("In-flight fence %d", i));
        instance.setDebugName(device.getHandle(), resources.commandPool.getHandle(), nstl::sprintf("Main %d", i));

        for (std::size_t index = 0; index < resources.commandBuffers.getSize(); index++)
            instance.setDebugName(device.getHandle(), resources.commandBuffers.getHandle(index), nstl::sprintf("Buffer%zu %d", index, i));

        m_frameResources.push_back(std::move(resources));
    }

    m_resourceManager = nstl::make_unique<ResourceManager>(device, physicalDevice, device.getGraphicsQueue(), *m_renderPass, FRAME_RESOURCE_COUNT);

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
    m_context->getDevice().waitIdle();

    destroySwapchain();
}

void vkgfx::Renderer::setCameraTransform(TestCameraTransform transform)
{
    m_cameraTransform = std::move(transform);
}

void vkgfx::Renderer::setCameraParameters(TestCameraParameters parameters)
{
    m_cameraParameters = std::move(parameters);
}

void vkgfx::Renderer::setLightParameters(TestLightParameters parameters)
{
    m_lightParameters = std::move(parameters);
}

void vkgfx::Renderer::addOneFrameTestObject(TestObject object)
{
    m_oneFrameTestObjects.push_back(std::move(object));
}

void vkgfx::Renderer::waitIdle()
{
    m_context->getDevice().waitIdle();
}

void vkgfx::Renderer::draw()
{
    static auto scopeId = memory::tracking::create_scope_id("Rendering/Draw");
    MEMORY_TRACKING_SCOPE(scopeId);

    RendererFrameResources& frameResources = m_frameResources[m_nextFrameResourcesIndex];

    frameResources.inFlightFence.wait();

    vko::Device const& device = m_context->getDevice();

    uint32_t imageIndex;
    VkResult aquireImageResult = vkAcquireNextImageKHR(device.getHandle(), m_swapchain->getHandle(), std::numeric_limits<uint64_t>::max(), frameResources.imageAvailableSemaphore.getHandle(), VK_NULL_HANDLE, &imageIndex);

    if (aquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }

    if (aquireImageResult != VK_SUCCESS && aquireImageResult != VK_SUBOPTIMAL_KHR)
        assert(false);

    frameResources.inFlightFence.reset();

    // TODO don't update camera buffer every frame
    // TODO update camera buffer directly without staging buffer
    updateCameraBuffer();
    m_resourceManager->transferDynamicBuffersFromStaging(m_nextFrameResourcesIndex);

    recordCommandBuffer(imageIndex, frameResources);
    frameResources.commandBuffers.submit(0, device.getGraphicsQueue(), &frameResources.renderFinishedSemaphore, &frameResources.imageAvailableSemaphore, &frameResources.inFlightFence);

    nstl::array waitSemaphores{ frameResources.renderFinishedSemaphore.getHandle() };
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
        assert(false);

    if (aquireImageResult == VK_SUBOPTIMAL_KHR)
        recreateSwapchain();

    m_nextFrameResourcesIndex = (m_nextFrameResourcesIndex + 1) % FRAME_RESOURCE_COUNT;
}

void vkgfx::Renderer::onWindowResized()
{
    m_window.waitUntilInForeground();

    m_context->onSurfaceChanged(); // TODO remove?

    recreateSwapchain();
}

void vkgfx::Renderer::createCameraResources()
{
    BufferMetadata metadata{
        .usage = vkgfx::BufferUsage::UniformBuffer,
        .location = vkgfx::BufferLocation::HostVisible,
        .isMutable = true,
    };
    m_cameraBuffer = m_resourceManager->createBuffer(sizeof(CameraData), std::move(metadata));

    DescriptorSetLayoutKey key = {
        .uniformConfig = {
            .hasBuffer = true,
            .hasAlbedoTexture = false,
            .hasNormalMap = false,
        },
    };

    m_frameDescriptorSetLayout = m_resourceManager->getOrCreateDescriptorSetLayout(key);
    m_data->frameDescriptorSetLayout = m_resourceManager->getDescriptorSetLayout(m_frameDescriptorSetLayout).getHandle();
    m_data->frameDescriptorPool.allocateRaw({ &m_data->frameDescriptorSetLayout, 1 }, { &m_data->frameDescriptorSet, 1 });

    {
        Buffer const* frameUniformBuffer = m_resourceManager->getBuffer(m_cameraBuffer);
        assert(frameUniformBuffer);

        DescriptorSetUpdateConfig config{
            .buffers = {
                {
                    .binding = 0,
                    .buffer = frameUniformBuffer->buffer.getHandle(),
                    .offset = 0,
                    .size = frameUniformBuffer->size,
                },
            },
        };

        updateDescriptorSet(m_context->getDevice().getHandle(), m_data->frameDescriptorSet, config, *m_cache);
    }
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

    nstl::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
    renderPassBeginInfo.pClearValues = clearValues.data();

    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 1.0f * m_width;
        viewport.height = 1.0f * m_height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    }

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    PipelineHandle boundPipeline;
    MaterialHandle boundMaterial;
    VkPipelineLayout currentPipelineLayoutForDescriptorSets = VK_NULL_HANDLE; // TODO use PipelineLayoutHandle, and change name

    auto drawTestObject = [this, commandBuffer, &frameResources, &boundPipeline, &boundMaterial, &currentPipelineLayoutForDescriptorSets](TestObject const& object)
    {
        vko::Pipeline const& pipeline = m_resourceManager->getPipeline(object.pipeline);

        if (boundPipeline != object.pipeline)
        {
            pipeline.bind(commandBuffer);
            boundPipeline = object.pipeline;
        }

        if (currentPipelineLayoutForDescriptorSets != pipeline.getPipelineLayoutHandle())
        {
            auto buffer = m_resourceManager->getBuffer(m_cameraBuffer);
            assert(buffer);
            auto frameUniformBufferOffset = static_cast<std::uint32_t>(buffer->getDynamicOffset(m_nextFrameResourcesIndex));
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayoutHandle(), 0, 1, &m_data->frameDescriptorSet, 1, &frameUniformBufferOffset);

            currentPipelineLayoutForDescriptorSets = pipeline.getPipelineLayoutHandle();
        }

        if (!object.pushConstants.empty())
            vkCmdPushConstants(commandBuffer, pipeline.getPipelineLayoutHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, object.pushConstants.size(), object.pushConstants.data()); // TODO configure shader stage

        if (object.material && boundMaterial != object.material)
        {
            nstl::span<VkDescriptorSetLayout const> descriptorSetLayouts = pipeline.getDescriptorSetLayouts();

            // TODO get rid of this hack
            assert(descriptorSetLayouts.size() == 3);
            assert(descriptorSetLayouts[0] == m_data->frameDescriptorSetLayout);
            descriptorSetLayouts = descriptorSetLayouts.subspan(1);

            nstl::vector<vko::DescriptorPool>& descriptorPools = frameResources.descriptorPools;

            m_cache->descriptorSets.resize(descriptorSetLayouts.size());
            bool descriptorSetsInitialized = false;
            do
            {
                if (!descriptorPools.empty())
                    descriptorSetsInitialized = descriptorPools.back().allocateRaw(descriptorSetLayouts, m_cache->descriptorSets);

                if (!descriptorSetsInitialized)
                    descriptorPools.emplace_back(m_context->getDevice());
            } while (!descriptorSetsInitialized);

            Material const* material = m_resourceManager->getMaterial(object.material);
            assert(material);

            // TODO refactor this hack
            m_cache->config1.clear();
            m_cache->config2.clear();
            m_cache->dynamicBufferOffsets12.clear();

            m_cache->config1.buffers.reserve(1);
            m_cache->config1.images.reserve(2);
            m_cache->config2.buffers.reserve(1);

            if (material->uniformBuffer)
            {
                Buffer const* materialUniformBuffer = m_resourceManager->getBuffer(material->uniformBuffer);
                assert(materialUniformBuffer);

                m_cache->config1.buffers.push_back({
                    .binding = 0,
                    .buffer = materialUniformBuffer->buffer.getHandle(),
                    .offset = 0,
                    .size = materialUniformBuffer->size,
                });

                m_cache->dynamicBufferOffsets12.push_back(static_cast<uint32_t>(materialUniformBuffer->getDynamicOffset(m_nextFrameResourcesIndex)));
            }

            if (object.uniformBuffer)
            {
                Buffer const* objectUniformBuffer = m_resourceManager->getBuffer(object.uniformBuffer);
                assert(objectUniformBuffer);

                m_cache->config2.buffers.push_back({
                    .binding = 0,
                    .buffer = objectUniformBuffer->buffer.getHandle(),
                    .offset = 0,
                    .size = objectUniformBuffer->size,
                });

                m_cache->dynamicBufferOffsets12.push_back(static_cast<uint32_t>(objectUniformBuffer->getDynamicOffset(m_nextFrameResourcesIndex)));
            }

            if (material->albedo)
            {
                Texture const* albedoTexture = m_resourceManager->getTexture(material->albedo);
                assert(albedoTexture);
                Image const* albedoImage = m_resourceManager->getImage(albedoTexture->image);
                assert(albedoImage);
                vko::Sampler const* albedoSampler = m_resourceManager->getSampler(albedoTexture->sampler);
                assert(albedoSampler);

                m_cache->config1.images.push_back({
                    .binding = 1,
                    .imageView = albedoImage->imageView.getHandle(),
                    .sampler = albedoSampler->getHandle(),
                });
            }

            if (material->normalMap)
            {
                Texture const* normalMapTexture = m_resourceManager->getTexture(material->normalMap);
                assert(normalMapTexture);
                Image const* normalMapImage = m_resourceManager->getImage(normalMapTexture->image);
                assert(normalMapImage);
                vko::Sampler const* normalMapSampler = m_resourceManager->getSampler(normalMapTexture->sampler);
                assert(normalMapSampler);

                m_cache->config1.images.push_back({
                    .binding = 2,
                    .imageView = normalMapImage->imageView.getHandle(),
                    .sampler = normalMapSampler->getHandle(),
                });
            }

            updateDescriptorSet(m_context->getDevice().getHandle(), m_cache->descriptorSets[0], m_cache->config1, *m_cache);
            updateDescriptorSet(m_context->getDevice().getHandle(), m_cache->descriptorSets[1], m_cache->config2, *m_cache);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayoutHandle(), 1, m_cache->descriptorSets.size(), m_cache->descriptorSets.data(), m_cache->dynamicBufferOffsets12.size(), m_cache->dynamicBufferOffsets12.data());

            boundMaterial = object.material;
        }

        Mesh const* mesh = m_resourceManager->getMesh(object.mesh);
        assert(mesh);

        m_cache->vertexBuffers.clear();
        m_cache->vertexBuffersOffsets.clear();
        m_cache->vertexBuffers.reserve(mesh->vertexBuffers.size());
        m_cache->vertexBuffersOffsets.reserve(mesh->vertexBuffers.size());
        for (BufferWithOffset const& bufferWithOffset : mesh->vertexBuffers)
        {
            Buffer const* vertexBuffer = m_resourceManager->getBuffer(bufferWithOffset.buffer);
            assert(vertexBuffer);
            m_cache->vertexBuffers.push_back(vertexBuffer->buffer.getHandle());
            m_cache->vertexBuffersOffsets.push_back(vertexBuffer->getDynamicOffset(m_nextFrameResourcesIndex) + bufferWithOffset.offset);
        }

        Buffer const* indexBuffer = m_resourceManager->getBuffer(mesh->indexBuffer.buffer);
        assert(indexBuffer);
        VkDeviceSize indexBufferOffset = indexBuffer->getDynamicOffset(m_nextFrameResourcesIndex) + mesh->indexBuffer.offset;

        VkRect2D scissor;
        if (object.hasScissors)
        {
            scissor.offset.x = object.scissorOffset.x;
            scissor.offset.y = object.scissorOffset.y;
            scissor.extent.width = object.scissorSize.x;
            scissor.extent.height = object.scissorSize.y;
        }
        else
        {
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = m_width;
            scissor.extent.height = m_height;
        }
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(m_cache->vertexBuffersOffsets.size()), m_cache->vertexBuffers.data(), m_cache->vertexBuffersOffsets.data());
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->buffer.getHandle(), indexBufferOffset, vulkanizeIndexType(mesh->indexType));

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->indexCount), 1, static_cast<uint32_t>(mesh->indexOffset), static_cast<uint32_t>(mesh->vertexOffset), 0);
    };

    for (TestObject const& object : m_oneFrameTestObjects)
        drawTestObject(object);

    m_oneFrameTestObjects.clear();

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

void vkgfx::Renderer::createSwapchain()
{
    vko::Instance const& instance = m_context->getInstance();
    vko::Device const& device = m_context->getDevice();

    vko::PhysicalDeviceSurfaceParameters const& parameters = m_context->getPhysicalDeviceSurfaceParameters();
    VkSurfaceCapabilitiesKHR const& capabilities = parameters.capabilities;
    nstl::span<VkPresentModeKHR const> presentModes = parameters.presentModes;
    vko::QueueFamily const& graphicsQueueFamily = *parameters.graphicsQueueFamily;
    vko::QueueFamily const& presentQueueFamily = *parameters.presentQueueFamily;

    VkExtent2D extent = chooseSwapchainExtent(m_context->getSurface(), capabilities);

    vko::Swapchain::Config config;
    config.surfaceFormat = m_data->m_surfaceFormat;
    config.presentMode = chooseSwapPresentMode(presentModes);
    config.extent = extent;

    const uint32_t minImageCount = capabilities.minImageCount;
    const uint32_t maxImageCount = capabilities.maxImageCount;
    config.minImageCount = minImageCount + 1;
    if (maxImageCount > 0)
        config.minImageCount = nstl::min(config.minImageCount, maxImageCount);

    config.preTransform = capabilities.currentTransform;

    m_swapchain = nstl::make_unique<vko::Swapchain>(device, m_context->getSurface(), graphicsQueueFamily, presentQueueFamily, std::move(config));
    instance.setDebugName(device.getHandle(), m_swapchain->getHandle(), "Main");

    // TODO move swapchain images to the ResourceManager?

    auto const& images = m_swapchain->getImages();
    m_swapchainImageViews.reserve(images.size());

    for (std::size_t i = 0; i < images.size(); i++)
    {
        vko::Image const& image = images[i];
        m_swapchainImageViews.push_back(nstl::make_unique<vko::ImageView>(device, image, VK_IMAGE_ASPECT_COLOR_BIT));

        instance.setDebugName(device.getHandle(), image.getHandle(), nstl::sprintf("Swapchain %zu", i));
        instance.setDebugName(device.getHandle(), m_swapchainImageViews.back()->getHandle(), nstl::sprintf("Swapchain %zu", i));
    }

    VkExtent2D swapchainExtent = m_swapchain->getExtent();

    m_depthImage = nstl::make_unique<vko::Image>(m_context->getDevice(), swapchainExtent.width, swapchainExtent.height, m_data->m_depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthImageMemory = nstl::make_unique<vko::DeviceMemory>(m_context->getDevice(), m_context->getPhysicalDevice(), m_depthImage->getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_depthImage->bindMemory(*m_depthImageMemory);

    m_depthImageView = nstl::make_unique<vko::ImageView>(device, *m_depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);

    instance.setDebugName(device.getHandle(), m_depthImage->getHandle(), "Main depth");
    instance.setDebugName(device.getHandle(), m_depthImageView->getHandle(), "Main depth");

    m_swapchainFramebuffers.reserve(m_swapchainImageViews.size());
    for (std::size_t i = 0; i < m_swapchainImageViews.size(); i++)
    {
        nstl::unique_ptr<vko::ImageView> const& colorImageView = m_swapchainImageViews[i];
        m_swapchainFramebuffers.push_back(nstl::make_unique<vko::Framebuffer>(device, *colorImageView, *m_depthImageView, *m_renderPass, m_swapchain->getExtent()));

        instance.setDebugName(device.getHandle(), m_swapchainFramebuffers.back()->getHandle(), nstl::sprintf("Main %zu", i));
    }

    m_width = extent.width;
    m_height = extent.height;
}

void vkgfx::Renderer::destroySwapchain()
{
    m_context->getDevice().waitIdle();

    m_swapchainFramebuffers.clear();
    m_depthImageView = nullptr;
    m_depthImage = nullptr;
    m_depthImageMemory = nullptr;
    m_swapchainImageViews.clear();
    m_swapchain = nullptr;
}

void vkgfx::Renderer::recreateSwapchain()
{
    destroySwapchain();
    createSwapchain();
}
