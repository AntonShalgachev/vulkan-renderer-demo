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
#include <array>
#include <chrono>
#include <iostream>
#include "Renderer.h"

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

    bool const VALIDATION_ENABLED = true;
    bool const API_DUMP_ENABLED = false;

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
        m_renderer->onFramebufferResized();

        m_application->onSurfaceChanged();
    }

    void initVulkan()
    {
        m_application = std::make_unique<vkr::Application>("Vulkan demo", m_window->getRequiredInstanceExtensions(), VALIDATION_ENABLED, API_DUMP_ENABLED, *m_window);

        m_descriptorSetLayout = std::make_unique<vkr::DescriptorSetLayout>(getApp());

        m_mesh1 = std::make_unique<vkr::Mesh>(getApp(), MODEL1_PATH);
        m_texture1 = std::make_unique<vkr::Texture>(getApp(), TEXTURE1_PATH);
        //m_mesh2 = std::make_unique<vkr::Mesh>(getApp(), MODEL2_PATH);
        //m_texture2 = std::make_unique<vkr::Texture>(getApp(), TEXTURE2_PATH);

        m_sampler = std::make_unique<vkr::Sampler>(getApp());

        m_renderer = std::make_unique<vkr::Renderer>(getApp(), *m_descriptorSetLayout);
        m_renderer->setUpdateUniformBufferCallback([this](uint32_t index) { updateUniformBuffer(index); });
        m_renderer->setOnSwapchainCreatedCallback([this](uint32_t imageCount) { onSwapchainCreated(imageCount); });
        m_renderer->setWaitUntilWindowInForegroundCallback([this]() { m_window->waitUntilInForeground(); });
    }

    void onSwapchainCreated(uint32_t imageCount)
    {
        m_instance1Model1 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), imageCount, *m_texture1, *m_sampler, *m_descriptorSetLayout);
        m_instance2Model1 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), imageCount, *m_texture1, *m_sampler, *m_descriptorSetLayout);
        //m_instance1Model2 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), m_swapchain->getImageCount(), *m_texture2, *m_sampler, *m_descriptorSetLayout);
        //m_instance2Model2 = std::make_unique<vkr::ObjectInstance>(getApp(), sizeof(UniformBufferObject), m_swapchain->getImageCount(), *m_texture2, *m_sampler, *m_descriptorSetLayout);

        m_renderer->setObjects(*m_mesh1, *m_instance1Model1, *m_instance2Model1);
    }

    void drawFrame()
    {
        m_renderer->draw();
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

        auto view = glm::lookAt(glm::vec3(0.0f, -3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        auto proj = glm::perspective(glm::radians(45.0f), m_renderer->getAspect(), 0.1f, 10.0f);
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

        getApp().getDevice().waitIdle();
    }

    vkr::Application const& getApp() { return *m_application; }

private:
    std::unique_ptr<vkr::Window> m_window;

    std::unique_ptr<vkr::Application> m_application;

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

    std::unique_ptr<vkr::Renderer> m_renderer;
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
        std::cerr << e.what() << std::endl;
        std::getchar();

        return EXIT_FAILURE;
    }

    // temporary to catch Vulkan errors
    std::getchar();
    return EXIT_SUCCESS;
}