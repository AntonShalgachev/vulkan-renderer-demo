#include "framework.h"
#include "Renderer.h"
#include "ServiceLocator.h"
#include "ImageView.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "Buffer.h"
#include "Sampler.h"
#include "ShaderModule.h"
#include "Swapchain.h"
#include <memory>
#include "RenderPass.h"
#include "Framebuffer.h"
#include "DescriptorSetLayout.h"
#include "PipelineLayout.h"
#include "Pipeline.h"
#include "Vertex.h"
#include "DescriptorSets.h"
#include "Mesh.h"
#include "Semaphore.h"
#include "Fence.h"
#include "CommandBuffers.h"
#include "ScopedOneTimeCommandBuffer.h"
#include "Utils.h"
#include "Texture.h"
#include "ObjectInstance.h"
#include "Window.h"
#include "Application.h"
#include "Device.h"
#include "Queue.h"

namespace
{
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    const uint32_t TARGET_WINDOW_WIDTH = 800;
    const uint32_t TARGET_WINDOW_HEIGHT = 600;

    const std::string MODEL1_PATH = "data/models/viking_room.obj";
    const std::string TEXTURE1_PATH = "data/textures/viking_room.png";
    const float MODEL1_SCALE = 1.0f;
    const float MODEL1_INSTANCE1_POSITION_X = -1.0f;
    const float MODEL1_INSTANCE1_AMPLITUDE_Z = 0.0f;
    const float MODEL1_INSTANCE2_POSITION_X = 1.0f;
    const float MODEL1_INSTANCE2_AMPLITUDE_Z = 0.0f;

    //const std::string MODEL2_PATH = "data/models/cat.obj";
    //const std::string TEXTURE2_PATH = "data/textures/cat.jpg";
    //const float MODEL2_SCALE = 0.01f;
    //const float MODEL2_INSTANCE1_POSITION_X = -1.0f;
    //const float MODEL2_INSTANCE1_AMPLITUDE_Z = 0.5f;
    //const float MODEL2_INSTANCE2_POSITION_x = 1.0f;
    //const float MODEL2_INSTANCE2_AMPLITUDE_Z = 0.5f;

    const int MAX_FRAMES_IN_FLIGHT = 2;
}

class HelloTriangleApplication
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
    }

private:
    void initWindow()
    {
        m_window = std::make_unique<vkr::Window>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");

        m_window->addResizeCallback([this](int, int) { onFramebufferResized(); });
    }

    void onFramebufferResized()
    {
        m_framebufferResized = true;

        vkr::temp::getRenderer()->onSurfaceChanged();
    }

    void initVulkan()
    {
        auto renderer = std::make_unique<vkr::Renderer>(*m_window);

        vkr::ServiceLocator::instance().setRenderer(std::move(renderer));

        m_descriptorSetLayout = std::make_unique<vkr::DescriptorSetLayout>(getApp());

        m_mesh1 = std::make_unique<vkr::Mesh>(getApp(), MODEL1_PATH);
        m_texture1 = std::make_unique<vkr::Texture>(getApp(), TEXTURE1_PATH);
        //m_mesh2 = std::make_unique<vkr::Mesh>(getApp(), MODEL2_PATH);
        //m_texture2 = std::make_unique<vkr::Texture>(getApp(), TEXTURE2_PATH);

        m_sampler = std::make_unique<vkr::Sampler>(getApp());

        initSwapchain();
        createSyncObjects();
    }

    void initSwapchain()
    {
        createSwapchain();
        m_renderPass = std::make_unique<vkr::RenderPass>(getApp(), *m_swapchain);
        createGraphicsPipeline();

        createDepthResources();
        m_swapchain->createFramebuffers(*m_renderPass, *m_depthImageView);

        m_instance1Model1 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), m_swapchain->getImageCount(), *m_texture1, *m_sampler, *m_descriptorSetLayout);
        m_instance2Model1 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), m_swapchain->getImageCount(), *m_texture1, *m_sampler, *m_descriptorSetLayout);
        //m_instance1Model2 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), m_swapchain->getImageCount(), *m_texture2, *m_sampler, *m_descriptorSetLayout);
        //m_instance2Model2 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), m_swapchain->getImageCount(), *m_texture2, *m_sampler, *m_descriptorSetLayout);

        createCommandBuffers();
    }

    void recreateSwapchain()
    {
        m_window->waitUntilInForeground();

        vkDeviceWaitIdle(getApp().getDevice().getHandle());

        initSwapchain();
    }

    void createSwapchain()
    {
        // TODO research why Vulkan crashes without this line
        m_swapchain = nullptr;

        m_swapchain = std::make_unique<vkr::Swapchain>(getApp());
    }

    void createGraphicsPipeline()
    {
        m_pipelineLayout = std::make_unique<vkr::PipelineLayout>(getApp(), *m_descriptorSetLayout);

        vkr::ShaderModule vertShaderModule{ getApp(), "data/shaders/vert.spv", vkr::ShaderModule::Type::Vertex, "main" };
        vkr::ShaderModule fragShaderModule{ getApp(), "data/shaders/frag.spv", vkr::ShaderModule::Type::Fragment, "main" };

        m_pipeline = std::make_unique<vkr::Pipeline>(getApp(), *m_pipelineLayout, *m_renderPass, m_swapchain->getExtent(), vertShaderModule, fragShaderModule);
    }

    void createDepthResources()
    {
        VkFormat depthFormat = m_renderPass->getDepthFormat();
        VkExtent2D swapchainExtent = m_swapchain->getExtent();

        vkr::utils::createImage(getApp(), swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
        m_depthImageView = m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void createCommandBuffers()
    {
        vkr::CommandPool const& commandPool = vkr::temp::getRenderer()->getCommandPool();
        vkr::Queue const& queue = getApp().getDevice().getGraphicsQueue();

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

            m_mesh1->bindBuffers(handle);

            m_instance1Model1->bindDescriptorSet(handle, i, *m_pipelineLayout);
            vkCmdDrawIndexed(handle, static_cast<uint32_t>(m_mesh1->getIndexCount()), 1, 0, 0, 0);

            m_instance2Model1->bindDescriptorSet(handle, i, *m_pipelineLayout);
            vkCmdDrawIndexed(handle, static_cast<uint32_t>(m_mesh1->getIndexCount()), 1, 0, 0, 0);

            //m_mesh2->bindBuffers(handle);

            //m_instance1Model2->bindDescriptorSet(handle, i, *m_pipelineLayout);
            //vkCmdDrawIndexed(handle, static_cast<uint32_t>(m_mesh2->getIndexCount()), 1, 0, 0, 0);

            //m_instance2Model2->bindDescriptorSet(handle, i, *m_pipelineLayout);
            //vkCmdDrawIndexed(handle, static_cast<uint32_t>(m_mesh2->getIndexCount()), 1, 0, 0, 0);

            vkCmdEndRenderPass(handle);

            m_commandBuffers->end(i);
        }
    }

    void createSyncObjects()
    {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_imageAvailableSemaphores.emplace_back(getApp());
            m_renderFinishedSemaphores.emplace_back(getApp());
            m_inFlightFences.emplace_back(getApp());
        }

        m_currentFences.resize(m_swapchain->getImageCount(), nullptr);
    }

    void drawFrame()
    {
        m_inFlightFences[m_currentFrame].wait();

        uint32_t imageIndex;
        VkResult aquireImageResult = vkAcquireNextImageKHR(getApp().getDevice().getHandle(), m_swapchain->getHandle(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame].getHandle(), VK_NULL_HANDLE, &imageIndex);

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

        m_commandBuffers->submit(imageIndex, m_renderFinishedSemaphores[m_currentFrame], m_imageAvailableSemaphores[m_currentFrame], m_inFlightFences[m_currentFrame]);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame].getHandle();

        VkSwapchainKHR swapchains[] = { m_swapchain->getHandle() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(getApp().getDevice().getPresentQueue().getHandle(), &presentInfo);

        if (aquireImageResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            recreateSwapchain();
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    UniformBufferObject createUbo(float time, float posX, float amplitudeZ, float scale, glm::mat4 const& view, glm::mat4 const& proj)
    {
        float posZ = amplitudeZ * (std::sin(5.0f * time) * 0.5f + 0.5f);
        glm::vec3 pos{ posX, 0.0f, posZ };

        UniformBufferObject ubo{};
        ubo.model = glm::translate(glm::mat4(1.0f), pos) * glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        ubo.view = view;
        ubo.proj = proj;

        return ubo;
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        VkExtent2D swapchainExtent = m_swapchain->getExtent();

        auto view = glm::lookAt(glm::vec3(0.0f, -3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        auto proj = glm::perspective(glm::radians(45.0f), 1.0f * swapchainExtent.width / swapchainExtent.height, 0.1f, 10.0f);
        proj[1][1] *= -1;

        auto ubo1Model1 = createUbo(time, MODEL1_INSTANCE1_POSITION_X, MODEL1_INSTANCE1_AMPLITUDE_Z, MODEL1_SCALE, view, proj);
        m_instance1Model1->copyToUniformBuffer(currentImage, &ubo1Model1, sizeof(ubo1Model1));

        auto ubo2Model1 = createUbo(time * 0.5f, MODEL1_INSTANCE2_POSITION_X, MODEL1_INSTANCE2_AMPLITUDE_Z, MODEL1_SCALE, view, proj);
        m_instance2Model1->copyToUniformBuffer(currentImage, &ubo2Model1, sizeof(ubo2Model1));

        //auto ubo1Model2 = createUbo(time, MODEL2_INSTANCE1_POSITION_X, MODEL2_INSTANCE1_AMPLITUDE_Z, MODEL2_SCALE, view, proj);
        //m_instance1Model2->copyToUniformBuffer(currentImage, &ubo1Model2, sizeof(ubo1Model2));

        //auto ubo2Model2 = createUbo(time * 0.5f, MODEL2_INSTANCE2_POSITION_x, MODEL2_INSTANCE2_AMPLITUDE_Z, MODEL2_SCALE, view, proj);
        //m_instance2Model2->copyToUniformBuffer(currentImage, &ubo2Model2, sizeof(ubo2Model2));
    }

    void mainLoop()
    {
        m_window->startEventLoop([this]() { drawFrame(); });

        vkDeviceWaitIdle(getApp().getDevice().getHandle());
    }

    vkr::Application const& getApp() { return vkr::temp::getRenderer()->getApplication(); }

private:
    std::unique_ptr<vkr::Window> m_window;

    // Renderer
    std::vector<vkr::Semaphore> m_imageAvailableSemaphores;
    std::vector<vkr::Semaphore> m_renderFinishedSemaphores;
    std::vector<vkr::Fence> m_inFlightFences;
    std::vector<vkr::Fence*> m_currentFences;

    std::size_t m_currentFrame = 0;
    bool m_framebufferResized = false;

    // Swapchain
    std::unique_ptr<vkr::Swapchain> m_swapchain;
    std::unique_ptr<vkr::RenderPass> m_renderPass;
    std::unique_ptr<vkr::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vkr::Pipeline> m_pipeline;

    std::unique_ptr<vkr::Image> m_depthImage;
    std::unique_ptr<vkr::DeviceMemory> m_depthImageMemory;
    std::unique_ptr<vkr::ImageView> m_depthImageView;

    std::unique_ptr<vkr::CommandBuffers> m_commandBuffers;

    // Per type
    std::unique_ptr<vkr::DescriptorSetLayout> m_descriptorSetLayout;

    // Per instance
    std::unique_ptr<vkr::ObjectInstance> m_instance1Model1;
    std::unique_ptr<vkr::ObjectInstance> m_instance2Model1;
    //std::unique_ptr<vkr::ObjectInstance> m_instance1Model2;
    //std::unique_ptr<vkr::ObjectInstance> m_instance2Model2;

    // Resources
    std::unique_ptr<vkr::Mesh> m_mesh1;
    std::unique_ptr<vkr::Texture> m_texture1;
    //std::unique_ptr<vkr::Mesh> m_mesh2;
    //std::unique_ptr<vkr::Texture> m_texture2;

    std::unique_ptr<vkr::Sampler> m_sampler;
};

int main()
{
    try
    {
        HelloTriangleApplication app;
        app.run();
    }
    catch (const std::exception& e)
    {
        vkr::ServiceLocator::instance().setRenderer(nullptr);

        std::cerr << e.what() << std::endl;
        std::getchar();

        return EXIT_FAILURE;
    }

    // temporary to catch Vulkan errors
    vkr::ServiceLocator::instance().setRenderer(nullptr);
    std::getchar();
    return EXIT_SUCCESS;
}