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

    //const std::string MODEL_PATH = "data/models/cat.obj";
    //const std::string TEXTURE_PATH = "data/textures/cat.jpg";
    //const float MODEL_SCALE = 0.05f;
    const std::string MODEL_PATH = "data/models/viking_room.obj";
    const std::string TEXTURE_PATH = "data/textures/viking_room.png";
    const float MODEL_SCALE = 1.0f;

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
        cleanup();
    }

private:
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo", nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    }

    static HelloTriangleApplication* getAppFromWindow(GLFWwindow* window)
    {
        return reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        if (auto app = getAppFromWindow(window))
            app->onFramebufferResized(width, height);
    }

    void onFramebufferResized(int width, int height)
    {
        m_framebufferResized = true;

        getRenderer()->OnSurfaceChanged(width, height);
    }

    void initVulkan()
    {
        auto renderer = std::make_unique<vkr::Renderer>(m_window);

        vkr::ServiceLocator::instance().setRenderer(std::move(renderer));

        m_descriptorSetLayout = std::make_unique<vkr::DescriptorSetLayout>();
        createTextureResources();
        loadModel();

        initSwapchain();
        createSyncObjects();
    }

    void initSwapchain()
    {
        createSwapchain();
        m_renderPass = std::make_unique<vkr::RenderPass>(*m_swapchain);
        createGraphicsPipeline();
        createDepthResources();
        m_swapchain->createFramebuffers(*m_renderPass, *m_depthImageView);
        createUniformBuffers();
        createDescriptorSets();
        createCommandBuffers();
    }

    void recreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(getDevice());

        initSwapchain();
    }

    void createSwapchain()
    {
        // TODO research why Vulkan crashes without this line
        m_swapchain = nullptr;

        m_swapchain = std::make_unique<vkr::Swapchain>();
    }

    void createGraphicsPipeline()
    {
        m_pipelineLayout = std::make_unique<vkr::PipelineLayout>(*m_descriptorSetLayout);

        vkr::ShaderModule vertShaderModule{ "data/shaders/vert.spv", vkr::ShaderModule::Type::Vertex, "main" };
        vkr::ShaderModule fragShaderModule{ "data/shaders/frag.spv", vkr::ShaderModule::Type::Fragment, "main" };

        m_pipeline = std::make_unique<vkr::Pipeline>(*m_pipelineLayout, *m_renderPass, m_swapchain->getExtent(), vertShaderModule, fragShaderModule);
    }

    void createDepthResources()
    {
        VkFormat depthFormat = m_renderPass->getDepthFormat();
        VkExtent2D swapchainExtent = m_swapchain->getExtent();

        createImage(swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
        m_depthImageView = m_depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void createTextureResources()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels)
            throw std::runtime_error("failed to load texture image!");

        size_t imageSize = texWidth * texHeight * 4;

        std::unique_ptr<vkr::Buffer> stagingBuffer;
        std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
        vkr::utils::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        stagingBufferMemory->copyFrom(pixels, imageSize);

        stbi_image_free(pixels);

        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

        transitionImageLayout(m_textureImage->getHandle(), m_textureImage->getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // TODO extract to some class
        copyBufferToImage(stagingBuffer->getHandle(), m_textureImage->getHandle(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(m_textureImage->getHandle(), m_textureImage->getFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_textureImageView = m_textureImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
        m_textureSampler = std::make_unique<vkr::Sampler>();
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        vkr::ScopedOneTimeCommandBuffer commandBuffer;

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer.getHandle(),
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        vkr::ScopedOneTimeCommandBuffer commandBuffer;

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer.getHandle(),
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vkr::Image>& image, std::unique_ptr<vkr::DeviceMemory>& imageMemory)
    {
        image = std::make_unique<vkr::Image>(width, height, format, tiling, usage);
        imageMemory = std::make_unique<vkr::DeviceMemory>(image->getMemoryRequirements(), properties);
        image->bind(*imageMemory);
    }

    void loadModel()
    {
        m_mesh = std::make_unique<vkr::Mesh>(MODEL_PATH);
    }

    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(m_swapchain->getImageCount());
        m_uniformBuffersMemory.resize(m_swapchain->getImageCount());

        for (size_t i = 0; i < m_swapchain->getImageCount(); i++)
            vkr::utils::createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
    }

    void createDescriptorSets()
    {
        m_descriptorSets = std::make_unique<vkr::DescriptorSets>(m_swapchain->getImageCount(), *m_descriptorSetLayout);

        for (size_t i = 0; i < m_descriptorSets->getSize(); i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i]->getHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_textureImageView->getHandle();
            imageInfo.sampler = m_textureSampler->getHandle();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            // TODO couple it with the data within DescriptorPool
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_descriptorSets->getHandles()[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_descriptorSets->getHandles()[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createCommandBuffers()
    {
        m_commandBuffers = std::make_unique<vkr::CommandBuffers>(m_swapchain->getImageCount());

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

            m_mesh->bindBuffers(handle);

            vkCmdBindDescriptorSets(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout->getHandle(), 0, 1, &m_descriptorSets->getHandles()[i], 0, nullptr);
            vkCmdDrawIndexed(handle, static_cast<uint32_t>(m_mesh->getIndexCount()), 1, 0, 0, 0);

            vkCmdEndRenderPass(handle);

            m_commandBuffers->end(i);
        }
    }

    void createSyncObjects()
    {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        m_currentFences.resize(m_swapchain->getImageCount(), nullptr);
    }

    void drawFrame()
    {
        m_inFlightFences[m_currentFrame].wait();

        uint32_t imageIndex;
        VkResult aquireImageResult = vkAcquireNextImageKHR(getDevice(), m_swapchain->getHandle(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame].getHandle(), VK_NULL_HANDLE, &imageIndex);

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

        vkQueuePresentKHR(getRenderer()->getPresentQueue(), &presentInfo);

        if (aquireImageResult == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            recreateSwapchain();
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        VkExtent2D swapchainExtent = m_swapchain->getExtent();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(MODEL_SCALE));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), 1.0f * swapchainExtent.width / swapchainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        m_uniformBuffersMemory[currentImage]->copyFrom(&ubo, sizeof(ubo));
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(getDevice());
    }

    void cleanup()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    std::unique_ptr<vkr::Renderer> const& getRenderer() const { return vkr::ServiceLocator::instance().getRenderer(); }
    VkDevice getDevice() const { return vkr::temp::getDevice(); }

private:
    GLFWwindow* m_window = nullptr;

    std::unique_ptr<vkr::Swapchain> m_swapchain;

    std::unique_ptr<vkr::RenderPass> m_renderPass;
    std::unique_ptr<vkr::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vkr::Pipeline> m_pipeline;

    std::unique_ptr<vkr::CommandBuffers> m_commandBuffers;

    std::vector<vkr::Semaphore> m_imageAvailableSemaphores;
    std::vector<vkr::Semaphore> m_renderFinishedSemaphores;
    std::vector<vkr::Fence> m_inFlightFences;

    std::vector<vkr::Fence*> m_currentFences;

    std::size_t m_currentFrame = 0;

    bool m_framebufferResized = false;

    std::vector<std::unique_ptr<vkr::Buffer>> m_uniformBuffers;
    std::vector<std::unique_ptr<vkr::DeviceMemory>> m_uniformBuffersMemory;

    std::unique_ptr<vkr::DescriptorSetLayout> m_descriptorSetLayout;
    std::unique_ptr<vkr::DescriptorSets> m_descriptorSets;

    std::unique_ptr<vkr::Image> m_textureImage;
    std::unique_ptr<vkr::DeviceMemory> m_textureImageMemory;
    std::unique_ptr<vkr::ImageView> m_textureImageView;
    std::unique_ptr<vkr::Sampler> m_textureSampler;

    std::unique_ptr<vkr::Image> m_depthImage;
    std::unique_ptr<vkr::DeviceMemory> m_depthImageMemory;
    std::unique_ptr<vkr::ImageView> m_depthImageView;

    std::unique_ptr<vkr::Mesh> m_mesh;
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