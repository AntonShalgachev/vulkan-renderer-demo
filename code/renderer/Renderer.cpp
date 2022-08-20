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

namespace
{
    struct UniformBufferObject
    {
        glm::mat4 projection;
        glm::vec3 lightPosition;
        glm::vec3 lightColor;
        glm::vec4 objectColor;
    };

    struct VertexPushConstants
    {
        glm::mat4 modelView;
        glm::mat4 normal;
    };

    const int FRAME_RESOURCE_COUNT = 3;
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

    DescriptorSetConfiguration config;
    config.hasTexture = material.getTexture() != nullptr;
    config.hasNormalMap = material.getNormalMap() != nullptr;

    auto const& resources = getUniformResources(config);

	m_drawableInstances.emplace_back(getApp(), *drawable, drawableObject.getTransform(), *resources.descriptorSetLayout, sizeof(UniformBufferObject), m_swapchain->getImages().size());
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

    FrameResources const& frameResources = m_frameResources[m_nextFrameResourcesIndex];

    uint32_t imageIndex;
    VkResult aquireImageResult = vkAcquireNextImageKHR(getDevice().getHandle(), m_swapchain->getHandle(), std::numeric_limits<uint64_t>::max(), frameResources.imageAvailableSemaphore.getHandle(), VK_NULL_HANDLE, &imageIndex);

    if (aquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }

    if (aquireImageResult != VK_SUCCESS && aquireImageResult != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swapchain image!");

    updateUniformBuffer(imageIndex);

    m_fenceTimer.start();
    frameResources.inFlightFence.wait();
    m_lastFenceTime = m_fenceTimer.getTime();
    m_cumulativeFenceTime += m_lastFenceTime;
    
    frameResources.inFlightFence.reset();

    recordCommandBuffer(imageIndex, frameResources);
    frameResources.commandBuffer.submit(getApp().getDevice().getGraphicsQueue(), &frameResources.renderFinishedSemaphore, &frameResources.imageAvailableSemaphore, &frameResources.inFlightFence);

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

    m_swapchain = std::make_unique<vkr::Swapchain>(getApp());
    m_renderPass = std::make_unique<vkr::RenderPass>(getApp(), *m_swapchain);

    auto const& images = m_swapchain->getImages();
    m_swapchainImageViews.reserve(images.size());
    for (auto const& image : images)
        m_swapchainImageViews.push_back(image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT));

    // TODO move depth resources to the swapchain?
    VkExtent2D swapchainExtent = m_swapchain->getExtent();
    vkr::utils::createImage(getApp(), swapchainExtent.width, swapchainExtent.height, m_renderPass->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    m_depthImageView = m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

    m_swapchainFramebuffers.reserve(m_swapchainImageViews.size());
    for (std::unique_ptr<vkr::ImageView> const& colorImageView : m_swapchainImageViews)
        m_swapchainFramebuffers.push_back(std::make_unique<vkr::Framebuffer>(getApp(), *colorImageView, *m_depthImageView, *m_renderPass, m_swapchain->getExtent()));

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

void vkr::Renderer::updateUniformBuffer(uint32_t currentImage)
{
	Camera* camera = m_activeCameraObject->getCamera();
	if (!camera)
		return;

	Transform const& cameraTransform = m_activeCameraObject->getTransform();

    for (ObjectInstance const& instance : m_drawableInstances)
    {
		Drawable const& drawable = instance.getDrawable();
        Transform const& transform = instance.getTransform();
        Material const& material = drawable.getMaterial();

        // TODO split into several buffers
        UniformBufferObject ubo{};
        ubo.projection = camera->getProjectionMatrix();
        ubo.lightPosition = cameraTransform.getViewMatrix() * glm::vec4(m_light->getTransform().getLocalPos(), 1.0f);
        ubo.lightColor = m_light->getColor() * m_light->getIntensity();
        ubo.objectColor = material.getColor();

        instance.copyToUniformBuffer(currentImage, &ubo, sizeof(ubo));
    }
}

void vkr::Renderer::updateCameraAspect()
{
    if (!m_activeCameraObject)
        return;

	if (auto const& camera = m_activeCameraObject->getCamera())
		camera->setAspect(getAspect());
}

vkr::Renderer::UniformResources const& vkr::Renderer::getUniformResources(DescriptorSetConfiguration const& config)
{
    auto it = m_uniformResources.find(config);

    if (it == m_uniformResources.end())
    {
        UniformResources resources;
        resources.descriptorSetLayout = std::make_unique<vkr::DescriptorSetLayout>(getApp(), config);
        resources.pipelineLayout = std::make_unique<vkr::PipelineLayout>(getApp(), *resources.descriptorSetLayout, sizeof(VertexPushConstants)); // TODO make push constants more configurable

        auto res = m_uniformResources.emplace(config, std::move(resources));
        it = res.first;
    }

    return it->second;
}

void vkr::Renderer::onSwapchainCreated()
{
    updateCameraAspect();
}

void vkr::Renderer::recordCommandBuffer(std::size_t imageIndex, FrameResources const& frameResources)
{
     // TODO remove hacks
    Pipeline::resetBoundPipeline();

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

    Transform const& cameraTransform = m_activeCameraObject->getTransform();

    for (ObjectInstance const& instance : m_drawableInstances)
    {
        Transform const& transform = instance.getTransform();

        Drawable const& drawable = instance.getDrawable();

        Mesh const& mesh = drawable.getMesh();
        Material const& material = drawable.getMaterial();

        DescriptorSetConfiguration config;
        config.hasTexture = material.getTexture() != nullptr;
        config.hasNormalMap = material.getNormalMap() != nullptr;

        auto const& resources = getUniformResources(config);

        // TODO don't create heavy configuration for each object instance
        vkr::PipelineConfiguration configuration;
        configuration.pipelineLayout = resources.pipelineLayout.get();
        configuration.renderPass = m_renderPass.get();
        configuration.extent = m_swapchain->getExtent();
        configuration.shaderKey = material.getShaderKey();
        configuration.vertexLayoutDescriptions = mesh.getVertexLayout().getDescriptions();
        configuration.cullBackFaces = !material.isDoubleSided();

        auto it = m_pipelines.find(configuration);
        if (it == m_pipelines.end())
            it = m_pipelines.emplace(configuration, createPipeline(configuration)).first;

        Pipeline const& pipeline = *it->second;

        pipeline.bind(handle);

        {
            VertexPushConstants constants;
            constants.modelView = cameraTransform.getViewMatrix() * transform.getMatrix();
            constants.normal = glm::transpose(glm::inverse(constants.modelView));

            vkCmdPushConstants(handle, resources.pipelineLayout->getHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushConstants), &constants);
        }

		instance.bindDescriptorSet(handle, imageIndex, *resources.pipelineLayout);

        mesh.draw(handle);
    }

    // TODO abstract this away
    if (ImGui::GetCurrentContext())
        if (ImDrawData* drawData = ImGui::GetDrawData())
            ImGui_ImplVulkan_RenderDrawData(drawData, handle);

    vkCmdEndRenderPass(handle);

    commandBuffer.end();
}

std::unique_ptr<vkr::Pipeline> vkr::Renderer::createPipeline(PipelineConfiguration const& configuration)
{
    return std::make_unique<vkr::Pipeline>(getApp(), configuration);
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
    : imageAvailableSemaphore(app)
    , renderFinishedSemaphore(app)
    , inFlightFence(app)
    , commandPool(std::make_unique<CommandPool>(app))
    , commandBuffer(commandPool->createCommandBuffer())
{
    app.setDebugName(imageAvailableSemaphore.getHandle(), "ImageAvailableSemaphore");
    app.setDebugName(renderFinishedSemaphore.getHandle(), "RenderFinishedSemaphore");
}
