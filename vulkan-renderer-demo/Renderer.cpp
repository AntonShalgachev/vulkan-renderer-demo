#include "Renderer.h"

#include <array>

#include "glm.h"

#include "Application.h"
#include "Device.h"
#include "CommandBuffers.h"
#include "Queue.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "Image.h"
#include "ImageView.h"
#include "DeviceMemory.h"
#include "PipelineLayout.h"
#include "Utils.h"
#include "ShaderModule.h"
#include "Mesh.h"
#include "ObjectInstance.h"
#include "SceneObject.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSets.h"
#include "Buffer.h"
#include "Material.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui.h"
#include "VertexLayout.h"

namespace
{
    struct UniformBufferObject
    {
        glm::mat4 modelView;
        glm::mat4 normal;
        glm::mat4 projection;
        glm::vec3 lightPosition;
        glm::vec3 viewPosition;
    };

    const int FRAME_RESOURCE_COUNT = 3;
}

vkr::Renderer::Renderer(Application const& app)
    : Object(app)
{
    // TODO define externally
    m_descriptorSetLayout = std::make_unique<vkr::DescriptorSetLayout>(getApp());

    createSwapchain();
    createSyncObjects();
}

vkr::Renderer::~Renderer() = default;

void vkr::Renderer::addObject(std::shared_ptr<SceneObject> const& object)
{
    m_sceneObjects.push_back(std::make_unique<vkr::ObjectInstance>(getApp(), object, *m_descriptorSetLayout, sizeof(UniformBufferObject)));
    m_sceneObjects.back()->onSwapchainCreated(*m_swapchain);
}

void vkr::Renderer::onFramebufferResized()
{
    m_framebufferResized = true;
}

void vkr::Renderer::draw()
{
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

    vkQueuePresentKHR(getDevice().getPresentQueue().getHandle(), &presentInfo); // TODO move to the object

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
    for (auto const& instance : m_sceneObjects)
        instance->setPipeline(nullptr);

    // TODO research why Vulkan crashes without this line
    m_swapchain = nullptr;

    m_swapchain = std::make_unique<vkr::Swapchain>(getApp());
    m_renderPass = std::make_unique<vkr::RenderPass>(getApp(), *m_swapchain);
    
    m_pipelineLayout = std::make_unique<vkr::PipelineLayout>(getApp(), *m_descriptorSetLayout);

    // TODO move depth resources to the swapchain?
    VkExtent2D swapchainExtent = m_swapchain->getExtent();
    vkr::utils::createImage(getApp(), swapchainExtent.width, swapchainExtent.height, m_renderPass->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    m_depthImageView = m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    m_swapchain->createFramebuffers(*m_renderPass, *m_depthImageView);

    onSwapchainCreated();
}

void vkr::Renderer::recreateSwapchain()
{
    if (m_waitUntilWindowInForeground)
        m_waitUntilWindowInForeground();

    getDevice().waitIdle();

    createSwapchain();
}

void vkr::Renderer::updateUniformBuffer(uint32_t currentImage)
{
    for (std::unique_ptr<vkr::ObjectInstance> const& instance : m_sceneObjects)
    {
        UniformBufferObject ubo{};
        ubo.modelView = m_camera.getViewMatrix() * instance->getSceneObject().getTransform().getMatrix();
        ubo.normal = glm::transpose(glm::inverse(ubo.modelView));
        ubo.projection = m_camera.getProjectionMatrix();
        ubo.lightPosition = m_camera.getViewMatrix() * glm::vec4(m_light->getTransform().getPos(), 1.0f);
        ubo.viewPosition = glm::vec3(0.0f, 0.0f, 0.0f); // TODO remove, always 0 in eye space

        instance->copyToUniformBuffer(currentImage, &ubo, sizeof(ubo));
    }
}

void vkr::Renderer::onSwapchainCreated()
{
    for (std::unique_ptr<vkr::ObjectInstance> const& instance : m_sceneObjects)
        instance->onSwapchainCreated(*m_swapchain);

    m_camera.setAspect(getAspect());
}

void vkr::Renderer::recordCommandBuffer(std::size_t imageIndex, FrameResources const& frameResources)
{
     // TODO remove hacks
    Mesh::resetBoundMesh();
    ObjectInstance::resetBoundDescriptorSet();
    Pipeline::resetBoundPipeline();

    frameResources.commandPool->reset();

    CommandBuffer const& commandBuffer = frameResources.commandBuffer;

    commandBuffer.begin(true);

    VkCommandBuffer handle = commandBuffer.getHandle();

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass->getHandle();
    renderPassBeginInfo.framebuffer = m_swapchain->getFramebuffers()[imageIndex]->getHandle(); // CRASH: get correct framebuffer
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = m_swapchain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (std::unique_ptr<vkr::ObjectInstance> const& instance : m_sceneObjects)
    {
        if (!instance->getSceneObject().isValid())
            continue;

        Mesh const& mesh = instance->getSceneObject().getMesh();
        Material const& material = instance->getSceneObject().getMaterial();
        Shader const& shader = *material.getShader();

        if (!instance->hasPipeline())
            instance->setPipeline(createPipeline(shader, mesh.getVertexLayout()));
        instance->bindPipeline(handle);

        mesh.bindBuffers(handle);

        instance->bindDescriptorSet(handle, imageIndex, *m_pipelineLayout);
        vkCmdDrawIndexed(handle, static_cast<uint32_t>(mesh.getIndexCount()), 1, 0, 0, 0);
    }

    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData)
        ImGui_ImplVulkan_RenderDrawData(drawData, handle);

    vkCmdEndRenderPass(handle);

    commandBuffer.end();
}

std::unique_ptr<vkr::Pipeline> vkr::Renderer::createPipeline(Shader const& shader, VertexLayout const& vertexLayout)
{
    return std::make_unique<vkr::Pipeline>(getApp(), *m_pipelineLayout, *m_renderPass, m_swapchain->getExtent(), shader, vertexLayout);
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

}
