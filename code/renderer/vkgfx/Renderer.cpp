#include "Renderer.h"

#include "renderer/wrapper/Swapchain.h"
#include "renderer/wrapper/Framebuffer.h"
#include "renderer/wrapper/RenderPass.h"
#include "renderer/wrapper/Image.h"
#include "renderer/wrapper/ImageView.h"
#include "renderer/wrapper/DeviceMemory.h"
#include "renderer/wrapper/PhysicalDevice.h"
#include "renderer/wrapper/Surface.h"
#include "renderer/wrapper/Semaphore.h"
#include "renderer/wrapper/Fence.h"
#include "renderer/wrapper/DescriptorPool.h"
#include "renderer/wrapper/CommandPool.h"
#include "renderer/wrapper/CommandBuffers.h"
#include "renderer/wrapper/Device.h"
#include "renderer/wrapper/Queue.h"
#include "renderer/wrapper/Pipeline.h"
#include "renderer/wrapper/Buffer.h"
#include "renderer/wrapper/Window.h"
#include "renderer/wrapper/DescriptorSetLayout.h"
#include "renderer/wrapper/Instance.h"
#include "renderer/wrapper/Sampler.h"

#include "renderer/vkgfx/ResourceManager.h"
#include "renderer/vkgfx/TestObject.h"
#include "renderer/vkgfx/Mesh.h"
#include "renderer/vkgfx/Buffer.h"
#include "renderer/vkgfx/Material.h"
#include "renderer/vkgfx/Texture.h"
#include "renderer/vkgfx/Image.h"
#include "renderer/vkgfx/PipelineKey.h"

// TODO remove vkr references
#include "renderer/Application.h"
#include "renderer/PhysicalDeviceSurfaceParameters.h"
#include "renderer/QueueFamilyIndices.h"

#include "glm.h"

#include "nstl/array.h"
#include "nstl/algorithm.h"
#include "nstl/sprintf.h"

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

    VkFormat findSupportedFormat(vko::PhysicalDevice const& physicalDevice, const nstl::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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
        static const nstl::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

        return findSupportedFormat(physicalDevice, candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const nstl::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.empty())
            return { VK_FORMAT_UNDEFINED , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const nstl::vector<VkPresentModeKHR>& availablePresentModes)
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
    };

    void updateDescriptorSet(VkDevice device, VkDescriptorSet set, DescriptorSetUpdateConfig const& config)
    {
        nstl::vector<VkDescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(config.buffers.size());

        nstl::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(config.images.size());

        nstl::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(bufferInfos.capacity() + imageInfos.capacity());

        for (DescriptorSetUpdateConfig::Buffer const& buffer : config.buffers)
        {
            VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

            assert(bufferInfos.size() < bufferInfos.capacity());
            VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
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
            assert(descriptorWrites.size() < descriptorWrites.capacity());
            VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

            assert(imageInfos.size() < imageInfos.capacity());
            VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
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

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
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
}

vkgfx::Renderer::Renderer(char const* name, bool enableValidationLayers, vko::Window& window, nstl::function<void(vko::DebugMessage)> onDebugMessage) : m_window(window)
{
    m_window.addResizeCallback([this](int, int) { onWindowResized(); });

    m_application = nstl::make_unique<vkr::Application>(name, enableValidationLayers, false, window, nstl::move(onDebugMessage));

    vko::Instance const& instance = m_application->getInstance();
    vko::Device const& device = m_application->getDevice();
    vko::PhysicalDevice const& physicalDevice = m_application->getPhysicalDevice();

    instance.setDebugName(device.getHandle(), device.getHandle(), "Device");
    instance.setDebugName(device.getHandle(), device.getGraphicsQueue().getHandle(), "Graphics");
    instance.setDebugName(device.getHandle(), device.getPresentQueue().getHandle(), "Present");
    instance.setDebugName(device.getHandle(), m_application->getShortLivedCommandPool().getHandle(), "Short-lived");

    vkr::PhysicalDeviceSurfaceParameters const& parameters = m_application->getPhysicalDeviceSurfaceParameters();

    m_data = nstl::make_unique<RendererData>(RendererData{
        .frameDescriptorPool{device},
    });

    m_data->m_surfaceFormat = chooseSwapSurfaceFormat(parameters.getFormats());
    m_data->m_depthFormat = findDepthFormat(physicalDevice);

    {
        m_renderPass = nstl::make_unique<vko::RenderPass>(device, m_data->m_surfaceFormat.format, m_data->m_depthFormat);
        instance.setDebugName(device.getHandle(), m_renderPass->getHandle(), "Main");
    }

    createSwapchain();

    vko::QueueFamily const& graphicsQueueFamily = m_application->getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getGraphicsQueueFamily();

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

        // TODO remove this ugly .c_str()
        instance.setDebugName(device.getHandle(), resources.imageAvailableSemaphore.getHandle(), nstl::sprintf("Image available %d", i).c_str());
        instance.setDebugName(device.getHandle(), resources.renderFinishedSemaphore.getHandle(), nstl::sprintf("Render finished %d", i).c_str());
        instance.setDebugName(device.getHandle(), resources.inFlightFence.getHandle(), nstl::sprintf("In-flight fence %d", i).c_str());
        instance.setDebugName(device.getHandle(), resources.commandPool.getHandle(), nstl::sprintf("Main %d", i).c_str());

        for (std::size_t index = 0; index < resources.commandBuffers.getSize(); index++)
            instance.setDebugName(device.getHandle(), resources.commandBuffers.getHandle(index), nstl::sprintf("Buffer{} %d", index, i).c_str());

        m_frameResources.push_back(std::move(resources));
    }

    m_resourceManager = nstl::make_unique<ResourceManager>(device, physicalDevice, m_application->getShortLivedCommandPool(), device.getGraphicsQueue(), *m_renderPass, FRAME_RESOURCE_COUNT);

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
    m_application->getDevice().waitIdle();
}

void vkgfx::Renderer::draw()
{
    RendererFrameResources& frameResources = m_frameResources[m_nextFrameResourcesIndex];

    frameResources.inFlightFence.wait();

    vko::Device const& device = m_application->getDevice();

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
    {
        recreateSwapchain();
    }

    m_nextFrameResourcesIndex = (m_nextFrameResourcesIndex + 1) % FRAME_RESOURCE_COUNT;
}

void vkgfx::Renderer::onWindowResized()
{
    m_window.waitUntilInForeground();

    m_application->onSurfaceChanged(); // TODO remove?

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

    m_frameUniformConfiguration = {
        .hasBuffer = true,
        .hasAlbedoTexture = false,
        .hasNormalMap = false,
    };

    DescriptorSetLayoutKey key = {
        .uniformConfig = m_frameUniformConfiguration,
    };

    m_frameDescriptorSetLayout = m_resourceManager->getOrCreateDescriptorSetLayout(key);
    m_data->frameDescriptorSetLayout = m_resourceManager->getDescriptorSetLayout(m_frameDescriptorSetLayout).getHandle();
    m_data->frameDescriptorSet = m_data->frameDescriptorPool.allocateRaw({ &m_data->frameDescriptorSetLayout, 1 })[0];

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

        updateDescriptorSet(m_application->getDevice().getHandle(), m_data->frameDescriptorSet, config);
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

            nstl::vector<VkDescriptorSet> descriptorSets;
            do
            {
                if (!descriptorPools.empty())
                    descriptorSets = descriptorPools.back().allocateRaw(descriptorSetLayouts);

                if (descriptorSets.empty())
                    descriptorPools.emplace_back(m_application->getDevice());
            } while (descriptorSets.empty());

            Material const* material = m_resourceManager->getMaterial(object.material);
            assert(material);

            DescriptorSetUpdateConfig config1;
            config1.buffers.reserve(1);
            config1.images.reserve(2);
            DescriptorSetUpdateConfig config2;
            config2.buffers.reserve(1);
            nstl::vector<uint32_t> dynamicBufferOffsets12;

            if (material->uniformBuffer)
            {
                Buffer const* materialUniformBuffer = m_resourceManager->getBuffer(material->uniformBuffer);
                assert(materialUniformBuffer);

                config1.buffers.push_back({
                    .binding = 0,
                    .buffer = materialUniformBuffer->buffer.getHandle(),
                    .offset = 0,
                    .size = materialUniformBuffer->size,
                });

                dynamicBufferOffsets12.push_back(static_cast<uint32_t>(materialUniformBuffer->getDynamicOffset(m_nextFrameResourcesIndex)));
            }

            if (object.uniformBuffer)
            {
                Buffer const* objectUniformBuffer = m_resourceManager->getBuffer(object.uniformBuffer);
                assert(objectUniformBuffer);

                config2.buffers.push_back({
                    .binding = 0,
                    .buffer = objectUniformBuffer->buffer.getHandle(),
                    .offset = 0,
                    .size = objectUniformBuffer->size,
                });

                dynamicBufferOffsets12.push_back(static_cast<uint32_t>(objectUniformBuffer->getDynamicOffset(m_nextFrameResourcesIndex)));
            }

            if (material->albedo)
            {
                Texture const* albedoTexture = m_resourceManager->getTexture(material->albedo);
                assert(albedoTexture);
                Image const* albedoImage = m_resourceManager->getImage(albedoTexture->image);
                assert(albedoImage);
                vko::Sampler const* albedoSampler = m_resourceManager->getSampler(albedoTexture->sampler);
                assert(albedoSampler);

                config1.images.push_back({
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

                config1.images.push_back({
                    .binding = 2,
                    .imageView = normalMapImage->imageView.getHandle(),
                    .sampler = normalMapSampler->getHandle(),
                });
            }

            updateDescriptorSet(m_application->getDevice().getHandle(), descriptorSets[0], config1);
            updateDescriptorSet(m_application->getDevice().getHandle(), descriptorSets[1], config2);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayoutHandle(), 1, descriptorSets.size(), descriptorSets.data(), dynamicBufferOffsets12.size(), dynamicBufferOffsets12.data());

            boundMaterial = object.material;
        }

        Mesh const* mesh = m_resourceManager->getMesh(object.mesh);
        assert(mesh);

        nstl::vector<VkBuffer> vertexBuffers;
        nstl::vector<VkDeviceSize> vertexBuffersOffsets;
        vertexBuffers.reserve(mesh->vertexBuffers.size());
        vertexBuffersOffsets.reserve(mesh->vertexBuffers.size());
        for (BufferWithOffset const& bufferWithOffset : mesh->vertexBuffers)
        {
            Buffer const* vertexBuffer = m_resourceManager->getBuffer(bufferWithOffset.buffer);
            assert(vertexBuffer);
            vertexBuffers.push_back(vertexBuffer->buffer.getHandle());
            vertexBuffersOffsets.push_back(vertexBuffer->getDynamicOffset(m_nextFrameResourcesIndex) + bufferWithOffset.offset);
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

        vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(vertexBuffersOffsets.size()), vertexBuffers.data(), vertexBuffersOffsets.data());
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
    vko::Instance const& instance = m_application->getInstance();
    vko::Device const& device = m_application->getDevice();
    vkr::PhysicalDeviceSurfaceParameters const& parameters = m_application->getPhysicalDeviceSurfaceParameters();
    vkr::QueueFamilyIndices const& indices = parameters.getQueueFamilyIndices();

    VkExtent2D extent = chooseSwapchainExtent(m_application->getSurface(), parameters.getCapabilities());

    vko::Swapchain::Config config;
    config.surfaceFormat = m_data->m_surfaceFormat;
    config.presentMode = chooseSwapPresentMode(parameters.getPresentModes());
    config.extent = extent;

    const uint32_t minImageCount = parameters.getCapabilities().minImageCount;
    const uint32_t maxImageCount = parameters.getCapabilities().maxImageCount;
    config.minImageCount = minImageCount + 1;
    if (maxImageCount > 0)
        config.minImageCount = nstl::min(config.minImageCount, maxImageCount);

    config.preTransform = parameters.getCapabilities().currentTransform;

    m_swapchain = nstl::make_unique<vko::Swapchain>(device, m_application->getSurface(), indices.getGraphicsQueueFamily(), indices.getPresentQueueFamily(), std::move(config));
    instance.setDebugName(device.getHandle(), m_swapchain->getHandle(), "Main");

    // TODO move swapchain images to the ResourceManager?

    auto const& images = m_swapchain->getImages();
    m_swapchainImageViews.reserve(images.size());

    for (std::size_t i = 0; i < images.size(); i++)
    {
        vko::Image const& image = images[i];
        m_swapchainImageViews.push_back(nstl::make_unique<vko::ImageView>(device, image, VK_IMAGE_ASPECT_COLOR_BIT));

        // TODO remove this ugly .c_str()
        instance.setDebugName(device.getHandle(), image.getHandle(), nstl::sprintf("Swapchain %d", i).c_str());
        instance.setDebugName(device.getHandle(), m_swapchainImageViews.back()->getHandle(), nstl::sprintf("Swapchain %d", i).c_str());
    }

    VkExtent2D swapchainExtent = m_swapchain->getExtent();

    m_depthImage = nstl::make_unique<vko::Image>(m_application->getDevice(), swapchainExtent.width, swapchainExtent.height, m_data->m_depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthImageMemory = nstl::make_unique<vko::DeviceMemory>(m_application->getDevice(), m_application->getPhysicalDevice(), m_depthImage->getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_depthImage->bindMemory(*m_depthImageMemory);

    m_depthImageView = nstl::make_unique<vko::ImageView>(device, *m_depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);

    instance.setDebugName(device.getHandle(), m_depthImage->getHandle(), "Main depth");
    instance.setDebugName(device.getHandle(), m_depthImageView->getHandle(), "Main depth");

    m_swapchainFramebuffers.reserve(m_swapchainImageViews.size());
    for (std::size_t i = 0; i < m_swapchainImageViews.size(); i++)
    {
        nstl::unique_ptr<vko::ImageView> const& colorImageView = m_swapchainImageViews[i];
        m_swapchainFramebuffers.push_back(nstl::make_unique<vko::Framebuffer>(device, *colorImageView, *m_depthImageView, *m_renderPass, m_swapchain->getExtent()));

        // TODO remove this ugly .c_str()
        instance.setDebugName(device.getHandle(), m_swapchainFramebuffers.back()->getHandle(), nstl::sprintf("Main %d", i).c_str());
    }

    m_width = extent.width;
    m_height = extent.height;
}

void vkgfx::Renderer::destroySwapchain()
{
    m_application->getDevice().waitIdle();

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
