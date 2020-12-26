#include "Renderer.h"
#include "Application.h"
#include "Device.h"
#include "CommandBuffers.h"
#include <array>
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

namespace
{
    const int MAX_FRAMES_IN_FLIGHT = 2;
}

vkr::Renderer::Renderer(Application const& app, vkr::DescriptorSetLayout const& descriptorSetLayout, vkr::ShaderModule const& vertexModule, vkr::ShaderModule const& fragmentModule)
    : Object(app)
    , m_descriptorSetLayout(descriptorSetLayout)
    , m_vertexShaderModule(vertexModule)
    , m_fragmentShaderModule(fragmentModule)
{
    createSwapchain();
    createSyncObjects();

    //createCommandBuffers();
}

vkr::Renderer::~Renderer() = default;

void vkr::Renderer::createSwapchain()
{
    // TODO research why Vulkan crashes without this line
    m_swapchain = nullptr;

    m_swapchain = std::make_unique<vkr::Swapchain>(getApp());
    m_renderPass = std::make_unique<vkr::RenderPass>(getApp(), *m_swapchain);
    
    m_pipelineLayout = std::make_unique<vkr::PipelineLayout>(getApp(), m_descriptorSetLayout);
    m_pipeline = std::make_unique<vkr::Pipeline>(getApp(), *m_pipelineLayout, *m_renderPass, m_swapchain->getExtent(), m_vertexShaderModule, m_fragmentShaderModule);

    VkExtent2D swapchainExtent = m_swapchain->getExtent();
    vkr::utils::createImage(getApp(), swapchainExtent.width, swapchainExtent.height, m_renderPass->getDepthFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    m_depthImageView = m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    m_swapchain->createFramebuffers(*m_renderPass, *m_depthImageView);
}

void vkr::Renderer::recreateSwapchain()
{
    getDevice().waitIdle();

    createSwapchain();
}

void vkr::Renderer::createCommandBuffers(Mesh const& mesh, ObjectInstance const& instance1, ObjectInstance const& instance2)
{
    vkr::CommandPool const& commandPool = getApp().getCommandPool();
    vkr::Queue const& queue = getDevice().getGraphicsQueue();

    m_commandBuffers = std::make_unique<vkr::CommandBuffers>(getApp(), commandPool, queue, m_swapchain->getImageCount());

    for (size_t i = 0; i < m_commandBuffers->getSize(); i++)
    {
        VkCommandBufferUsageFlags const flags = 0;
        m_commandBuffers->begin(i, flags);

        VkCommandBuffer handle = m_commandBuffers->getHandle(i);

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
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getHandle());

        mesh.bindBuffers(handle);

        instance1.bindDescriptorSet(handle, i, *m_pipelineLayout);
        vkCmdDrawIndexed(handle, static_cast<uint32_t>(mesh.getIndexCount()), 1, 0, 0, 0);

        instance2.bindDescriptorSet(handle, i, *m_pipelineLayout);
        vkCmdDrawIndexed(handle, static_cast<uint32_t>(mesh.getIndexCount()), 1, 0, 0, 0);

        //m_mesh2->bindBuffers(handle);

        //m_instance1Model2->bindDescriptorSet(handle, i, *m_pipelineLayout);
        //vkCmdDrawIndexed(handle, static_cast<uint32_t>(m_mesh2->getIndexCount()), 1, 0, 0, 0);

        //m_instance2Model2->bindDescriptorSet(handle, i, *m_pipelineLayout);
        //vkCmdDrawIndexed(handle, static_cast<uint32_t>(m_mesh2->getIndexCount()), 1, 0, 0, 0);

        vkCmdEndRenderPass(handle);

        m_commandBuffers->end(i);
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

void vkr::Renderer::drawFrame()
{
    //if (!m_commandBuffers)
    //    return;

    //m_inFlightFences[m_currentFrame].wait();

    //uint32_t imageIndex;
    //VkResult aquireImageResult = vkAcquireNextImageKHR(getDevice().getHandle(), m_swapchain->getHandle(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame].getHandle(), VK_NULL_HANDLE, &imageIndex);

    //if (aquireImageResult == VK_ERROR_OUT_OF_DATE_KHR)
    //{
    //    recreateSwapchain();
    //    return;
    //}

    //if (aquireImageResult != VK_SUCCESS && aquireImageResult != VK_SUBOPTIMAL_KHR)
    //    throw std::runtime_error("failed to acquire swapchain image!");

    //if (m_currentFences[imageIndex] != nullptr)
    //    m_currentFences[imageIndex]->wait();

    //m_currentFences[imageIndex] = &m_inFlightFences[m_currentFrame];

    //updateUniformBuffer(imageIndex);

    //m_inFlightFences[m_currentFrame].reset();

    //m_commandBuffers->submit(imageIndex, m_renderFinishedSemaphores[m_currentFrame], m_imageAvailableSemaphores[m_currentFrame], m_inFlightFences[m_currentFrame]);

    //VkPresentInfoKHR presentInfo{};
    //presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    //presentInfo.waitSemaphoreCount = 1;
    //presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame].getHandle();

    //VkSwapchainKHR swapchains[] = { m_swapchain->getHandle() };
    //presentInfo.swapchainCount = 1;
    //presentInfo.pSwapchains = swapchains;
    //presentInfo.pImageIndices = &imageIndex;
    //presentInfo.pResults = nullptr;

    //vkQueuePresentKHR(getDevice().getPresentQueue().getHandle(), &presentInfo);

    //if (aquireImageResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    //{
    //    m_framebufferResized = false;
    //    recreateSwapchain();
    //}

    //m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
