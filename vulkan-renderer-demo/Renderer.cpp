#include "Renderer.h"

#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

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
#include "Shader.h"
#include "Material.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "CommandPool.h"
#include "CommandBuffer.h"

namespace
{
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    const int MAX_FRAMES_IN_FLIGHT = 2;
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

void vkr::Renderer::addShader(Shader const& shader)
{
    auto pipeline = std::make_unique<vkr::Pipeline>(getApp(), *m_pipelineLayout, *m_renderPass, m_swapchain->getExtent(), shader.createVertexModule(), shader.createFragmentModule());
    m_pipelines.emplace(&shader, std::move(pipeline));
}

void vkr::Renderer::addObject(std::shared_ptr<SceneObject> const& object)
{
    m_sceneObjects.push_back(std::make_unique<vkr::ObjectInstance>(getApp(), object, *m_descriptorSetLayout, sizeof(UniformBufferObject)));
    m_sceneObjects.back()->onSwapchainCreated(*m_swapchain);
}

void vkr::Renderer::finalizeObjects()
{
    createCommandBuffers();
}

void vkr::Renderer::onFramebufferResized()
{
    m_framebufferResized = true;
}

void vkr::Renderer::draw()
{
    m_inFlightFences[m_currentFrame].wait();

    uint32_t imageIndex;
    VkResult aquireImageResult = vkAcquireNextImageKHR(getDevice().getHandle(), m_swapchain->getHandle(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame].getHandle(), VK_NULL_HANDLE, &imageIndex);

    if (aquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }

    if (aquireImageResult != VK_SUCCESS && aquireImageResult != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swapchain image!");

    if (m_currentFences[imageIndex] != nullptr)
        m_currentFences[imageIndex]->wait();

    m_currentFences[imageIndex] = &m_inFlightFences[m_currentFrame];

    updateUniformBuffer(imageIndex);

    m_inFlightFences[m_currentFrame].reset();

    m_commandBuffers[imageIndex].submit(getApp().getDevice().getGraphicsQueue(), m_renderFinishedSemaphores[m_currentFrame], m_imageAvailableSemaphores[m_currentFrame], m_inFlightFences[m_currentFrame]);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame].getHandle();

    VkSwapchainKHR swapchains[] = { m_swapchain->getHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(getDevice().getPresentQueue().getHandle(), &presentInfo);

    if (aquireImageResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        recreateSwapchain();
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

float vkr::Renderer::getAspect() const
{
    VkExtent2D extent = m_swapchain->getExtent();

    return 1.0f * extent.width / extent.height;
}

void vkr::Renderer::createSwapchain()
{
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
    createCommandBuffers();
}

void vkr::Renderer::updateUniformBuffer(uint32_t currentImage)
{
    for (std::unique_ptr<vkr::ObjectInstance> const& instance : m_sceneObjects)
    {
        // TODO extract to Camera object
        auto view = glm::lookAt(glm::vec3(0.0f, -3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        auto proj = glm::perspective(glm::radians(45.0f), getAspect(), 0.1f, 10.0f);
        proj[1][1] *= -1;

        UniformBufferObject ubo{};
        ubo.model = instance->getSceneObject().getMatrix();
        ubo.view = view;
        ubo.proj = proj;

        instance->copyToUniformBuffer(currentImage, &ubo, sizeof(ubo));
    }
}

void vkr::Renderer::onSwapchainCreated()
{
    for (std::unique_ptr<vkr::ObjectInstance> const& instance : m_sceneObjects)
        instance->onSwapchainCreated(*m_swapchain);
}

void vkr::Renderer::createCommandBuffers()
{
    m_commandPool = std::make_unique<vkr::CommandPool>(getApp());

    m_commandBuffers = m_commandPool->createCommandBuffers(m_swapchain->getImageCount());

    for (size_t i = 0; i < m_commandBuffers.size(); i++)
    {
        CommandBuffer& commandBuffer = m_commandBuffers[i];

        VkCommandBufferUsageFlags const flags = 0;
        commandBuffer.begin(flags);

        VkCommandBuffer handle = commandBuffer.getHandle();

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_renderPass->getHandle();
        renderPassBeginInfo.framebuffer = m_swapchain->getFramebuffers()[i]->getHandle();
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
            Material const& material = instance->getSceneObject().getMaterial();
            Shader const& shader = material.getShader();

            auto it = m_pipelines.find(&shader);
            if (it == m_pipelines.end())
                throw std::runtime_error("Failed to find a pipeline for the shader");

            // TODO bind only if the shader changed
            it->second->bind(handle);

            Mesh const& mesh = instance->getSceneObject().getMesh();

            // TODO bind only if it's not already bound
            mesh.bindBuffers(handle);

            instance->bindDescriptorSet(handle, i, *m_pipelineLayout);
            vkCmdDrawIndexed(handle, static_cast<uint32_t>(mesh.getIndexCount()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(handle);

        commandBuffer.end();
    }
}

void vkr::Renderer::createSyncObjects()
{
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_imageAvailableSemaphores.emplace_back(getApp());
        m_renderFinishedSemaphores.emplace_back(getApp());
        m_inFlightFences.emplace_back(getApp());
    }

    m_currentFences.resize(m_swapchain->getImageCount(), nullptr);
}
