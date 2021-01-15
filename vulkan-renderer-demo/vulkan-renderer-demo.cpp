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
#include "Material.h"
#include "SceneObject.h"

namespace
{

    const uint32_t TARGET_WINDOW_WIDTH = 800;
    const uint32_t TARGET_WINDOW_HEIGHT = 600;

    bool const VALIDATION_ENABLED = true;
    bool const API_DUMP_ENABLED = false;

    const std::string MODEL_ROOM_PATH = "data/models/viking_room.obj";
    const std::string TEXTURE_ROOM_PATH = "data/textures/viking_room.png";
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
    HelloTriangleApplication()
    {
        m_window = std::make_unique<vkr::Window>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
        m_window->addResizeCallback([this](int, int) { onFramebufferResized(); });
        m_application = std::make_unique<vkr::Application>("Vulkan demo", m_window->getRequiredInstanceExtensions(), VALIDATION_ENABLED, API_DUMP_ENABLED, *m_window);

        loadResources();
        createRenderer();
        createSceneObjects();
    }

    void run()
    {
        m_window->startEventLoop([this]() { drawFrame(); });

        getApp().getDevice().waitIdle();
    }

private:
    void onFramebufferResized()
    {
        m_renderer->onFramebufferResized();
        m_application->onSurfaceChanged();
    }

    void createRenderer()
    {
        m_renderer = std::make_unique<vkr::Renderer>(getApp());
        m_renderer->setWaitUntilWindowInForegroundCallback([this]() { m_window->waitUntilInForeground(); });
    }

    void loadResources()
    {
        auto defaultSampler = std::make_shared<vkr::Sampler>(getApp());

        m_roomMesh = std::make_shared<vkr::Mesh>(getApp(), MODEL_ROOM_PATH);

        auto roomTexture = std::make_shared<vkr::Texture>(getApp(), TEXTURE_ROOM_PATH);
        m_roomMaterial = std::make_shared<vkr::Material>();
        m_roomMaterial->setTexture(roomTexture);
        m_roomMaterial->setSampler(defaultSampler);

        //m_mesh2 = std::make_unique<vkr::Mesh>(getApp(), MODEL2_PATH);
        //m_texture2 = std::make_unique<vkr::Texture>(getApp(), TEXTURE2_PATH);
    }

    void createSceneObjects()
    {
        m_leftRoom = std::make_shared<vkr::SceneObject>();
        m_leftRoom->setMesh(m_roomMesh);
        m_leftRoom->setMaterial(m_roomMaterial);

        m_rightRoom = std::make_shared<vkr::SceneObject>();
        m_rightRoom->setMesh(m_roomMesh);
        m_rightRoom->setMaterial(m_roomMaterial);

        m_renderer->addObject(m_leftRoom);
        m_renderer->addObject(m_rightRoom);
        m_renderer->finalizeObjects();
    }

    void drawFrame()
    {
        update();
        m_renderer->draw();
    }

    void update()
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        auto updateObject = [](vkr::SceneObject& object, float time, float posX, float amplitudeZ, float scale)
        {
            float posZ = amplitudeZ * (std::sin(5.0f * time) * 0.5f + 0.5f);
            object.setPos({ posX, 0.0f, posZ });

            object.setRotation(glm::angleAxis(time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

            object.setScale(glm::vec3(scale));
        };

        updateObject(*m_leftRoom, time, MODEL1_INSTANCE1_POSITION_X, MODEL1_INSTANCE1_AMPLITUDE_Z, MODEL1_SCALE);
        updateObject(*m_rightRoom, time * 0.5f, MODEL1_INSTANCE2_POSITION_X, MODEL1_INSTANCE2_AMPLITUDE_Z, MODEL1_SCALE);
    }

    vkr::Application const& getApp() { return *m_application; }

private:
    std::unique_ptr<vkr::Window> m_window;

    std::unique_ptr<vkr::Application> m_application;
    std::unique_ptr<vkr::Renderer> m_renderer;

    // Resources
    std::shared_ptr<vkr::Mesh> m_roomMesh;
    //std::shared_ptr<vkr::Texture> m_texture1;
    std::shared_ptr<vkr::Material> m_roomMaterial;
    //std::unique_ptr<vkr::Mesh> m_mesh2;
    //std::unique_ptr<vkr::Texture> m_texture2;

    // Objects
    std::shared_ptr<vkr::SceneObject> m_leftRoom;
    std::shared_ptr<vkr::SceneObject> m_rightRoom;
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