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
#include <iostream>
#include "Renderer.h"
#include "Material.h"
#include "SceneObject.h"
#include "Shader.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "DescriptorPool.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Timer.h"

#include <tiny_gltf.h>

namespace
{
    const uint32_t TARGET_WINDOW_WIDTH = 800;
    const uint32_t TARGET_WINDOW_HEIGHT = 600;

    const float FPS_PERIOD = 0.2f;

#ifdef _DEBUG
    bool const VALIDATION_ENABLED = true;
#else
    bool const VALIDATION_ENABLED = false;
#endif
    bool const API_DUMP_ENABLED = false;

    const std::string MODEL_ROOM_PATH = "data/meshes/viking_room.obj";
    const std::string TEXTURE_ROOM_PATH = "data/textures/viking_room.png";
    const float MODEL1_SCALE = 1.0f;
    const float MODEL1_INSTANCE1_POSITION_X = -1.0f;
    const float MODEL1_INSTANCE1_AMPLITUDE_Z = 0.0f;
    const float MODEL1_INSTANCE2_POSITION_X = 1.0f;
    const float MODEL1_INSTANCE2_AMPLITUDE_Z = 0.0f;

    const std::string GLTF_MODEL_PATH = "data/models/Duck/glTF/Duck.gltf";
    const float GLTF_MODEL_SCALE = 0.01f;

    std::shared_ptr<tinygltf::Model> loadModel(std::string const& path)
    {
        std::shared_ptr<tinygltf::Model> model = std::make_shared<tinygltf::Model>();
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        bool success = loader.LoadASCIIFromFile(model.get(), &err, &warn, path);

        if (!warn.empty())
            std::cout << warn << std::endl;
        if (!err.empty())
            std::cout << err << std::endl;

        if (!success)
            return nullptr;

        return model;
    }
}

class HelloTriangleApplication
{
public:
    HelloTriangleApplication()
    {
        m_window = std::make_unique<vkr::Window>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
        m_window->addResizeCallback([this](int, int) { onFramebufferResized(); });
        m_application = std::make_unique<vkr::Application>("Vulkan demo", VALIDATION_ENABLED, API_DUMP_ENABLED, *m_window);

        loadResources();
        createRenderer();
        initImGui();

        createSceneObjects();
    }

    ~HelloTriangleApplication()
    {
        getApp().getDevice().waitIdle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void run()
    {
        m_frameTimer.start();
        m_window->startEventLoop([this]() { drawFrame(); });
    }

private:
    void initImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        io.Fonts->AddFontDefault();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(m_window->getHandle(), true);

        m_descriptorPool = std::make_unique<vkr::DescriptorPool>(getApp(), 1);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = getApp().getInstance().getHandle();
        init_info.PhysicalDevice = getApp().getPhysicalDevice().getHandle();
        init_info.Device = getApp().getDevice().getHandle();
        init_info.QueueFamily = getApp().getDevice().getGraphicsQueue().getFamily().getIndex();
        init_info.Queue = getApp().getDevice().getGraphicsQueue().getHandle();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = m_descriptorPool->getHandle();
        init_info.Allocator = nullptr;
        init_info.MinImageCount = 2; // TOO fetch?
        init_info.ImageCount = static_cast<uint32_t>(m_renderer->getSwapchain().getImageCount());
        init_info.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&init_info, m_renderer->getRenderPass().getHandle());

        {
            vkr::ScopedOneTimeCommandBuffer buffer{ getApp() };
            ImGui_ImplVulkan_CreateFontsTexture(buffer.getHandle());
            buffer.submit();

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void onFramebufferResized()
    {
        m_renderer->onFramebufferResized();
        m_application->onSurfaceChanged();
    }

    void createRenderer()
    {
        m_renderer = std::make_unique<vkr::Renderer>(getApp());
        m_renderer->setWaitUntilWindowInForegroundCallback([this]() { m_window->waitUntilInForeground(); });

        updateCamera();
    }

    void loadResources()
    {
        m_defaultShader = std::make_shared<vkr::Shader>(getApp(), "data/shaders/default.vert.spv", "data/shaders/default.frag.spv");
        m_noColorShader = std::make_shared<vkr::Shader>(getApp(), "data/shaders/default.vert.spv", "data/shaders/no-color.frag.spv");

        m_defaultSampler = std::make_shared<vkr::Sampler>(getApp());

        m_roomMesh = std::make_shared<vkr::Mesh>(getApp(), MODEL_ROOM_PATH);

        auto roomTexture = std::make_shared<vkr::Texture>(getApp(), TEXTURE_ROOM_PATH);
        m_roomMaterial = std::make_shared<vkr::Material>();
        m_roomMaterial->setTexture(roomTexture);
        m_roomMaterial->setSampler(m_defaultSampler);
        m_roomMaterial->setShader(m_defaultShader);

        m_grayscaleRoomMaterial = std::make_shared<vkr::Material>();
        m_grayscaleRoomMaterial->setTexture(roomTexture);
        m_grayscaleRoomMaterial->setSampler(m_defaultSampler);
        m_grayscaleRoomMaterial->setShader(m_noColorShader);

        m_gltfModel = loadModel(GLTF_MODEL_PATH);
    }

    std::unique_ptr<vkr::SceneObject> createSceneObject(std::shared_ptr<tinygltf::Model> const& model)
    {
        std::unique_ptr<vkr::SceneObject> object = std::make_unique<vkr::SceneObject>();

        std::size_t const meshIndex = 0;
        std::size_t const primitiveIndex = 0;

        object->setMesh(std::make_shared<vkr::Mesh>(getApp(), model, meshIndex, primitiveIndex));

        auto material = std::make_shared<vkr::Material>();
        material->setShader(m_noColorShader);

        // TODO choose shader properly
        if (!model->images.empty())
        {
            tinygltf::Image const& image = model->images[0];
            auto texture = std::make_shared<vkr::Texture>(getApp(), image);
            material->setTexture(texture);
            material->setSampler(m_defaultSampler);
            material->setShader(m_defaultShader);
        }

        object->setMaterial(material);

        return object;
    }

    void createSceneObjects()
    {
        m_leftRoom = std::make_shared<vkr::SceneObject>();
        m_leftRoom->setMesh(m_roomMesh);
        m_leftRoom->setMaterial(m_roomMaterial);

        m_rightRoom = std::make_shared<vkr::SceneObject>();
        m_rightRoom->setMesh(m_roomMesh);
        m_rightRoom->setMaterial(m_grayscaleRoomMaterial);

        m_renderer->addObject(m_leftRoom);
        //m_renderer->addObject(m_rightRoom);

        m_model = createSceneObject(m_gltfModel);
        m_renderer->addObject(m_model);
    }

    void updateUI(float frameTime, float fenceTime)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float cpuUtilizationRatio = 1.0f - fenceTime/frameTime;

        ImGui::Begin("Time", nullptr);
        ImGui::Text("Frame time %.3f ms", frameTime * 1000.0f);
        ImGui::Text("Fence time %.3f ms", fenceTime * 1000.0f);
        ImGui::Text("CPU Utilization %.2f%%", cpuUtilizationRatio * 100.0f);
        ImGui::End();

        ImGui::Begin("Camera");
        ImGui::SliderFloat3("Position", &m_cameraPos[0], -10.0f, 10.0f, "%.2f", 1.0f);
        ImGui::SliderFloat3("Rotation", &m_cameraRotation[0], -180.0f, 180.0f, "%.1f", 1.0f);
        ImGui::End();

        ImGui::Render();
    }

    void drawFrame()
    {
        update();
        m_renderer->draw();
        m_fpsDrawnFrames++;
    }

    void update()
    {
        if (m_frameTimer.getTime() > FPS_PERIOD)
        {
            m_lastFrameTime = m_frameTimer.loop() / m_fpsDrawnFrames;
            m_lastFenceTime = m_renderer->getCumulativeFenceTime() / m_fpsDrawnFrames;
            m_renderer->resetCumulativeFenceTime();
            m_fpsDrawnFrames = 0;
        }

        updateUI(m_lastFrameTime, m_lastFenceTime);

        auto updateObject = [](vkr::Transform& transform, float time, float posX, float amplitudeZ, float scale, bool fixRotation)
        {
            float posZ = amplitudeZ * (std::sin(5.0f * time) * 0.5f + 0.5f);
            transform.setPos({ posX, 0.0f, posZ });

            glm::quat initialRotation = glm::identity<glm::quat>();
            if (fixRotation)
                initialRotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            transform.setRotation(glm::angleAxis(time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * initialRotation);

            transform.setScale(glm::vec3(scale));
        };

        float time = m_appTime.getTime();
        updateObject(m_leftRoom->getTransform(), time, MODEL1_INSTANCE1_POSITION_X, MODEL1_INSTANCE1_AMPLITUDE_Z, MODEL1_SCALE, false);
        updateObject(m_rightRoom->getTransform(), time * 0.5f, MODEL1_INSTANCE2_POSITION_X, MODEL1_INSTANCE2_AMPLITUDE_Z, MODEL1_SCALE, false);

         updateObject(m_model->getTransform(), time * 0.5f, MODEL1_INSTANCE2_POSITION_X, MODEL1_INSTANCE2_AMPLITUDE_Z, GLTF_MODEL_SCALE, true);

        updateCamera();
    }

    void updateCamera()
    {
        m_renderer->getCamera().getTransform().setPos(m_cameraPos);

        m_renderer->getCamera().getTransform().setRotation(glm::radians(m_cameraRotation));
    }

    vkr::Application const& getApp() { return *m_application; }

private:
    std::unique_ptr<vkr::Window> m_window;

    std::unique_ptr<vkr::Application> m_application;
    std::unique_ptr<vkr::Renderer> m_renderer;

    // Resources
    std::shared_ptr<vkr::Sampler> m_defaultSampler;
    std::shared_ptr<vkr::Shader> m_defaultShader;
    std::shared_ptr<vkr::Shader> m_noColorShader;

    std::shared_ptr<vkr::Mesh> m_roomMesh;
    std::shared_ptr<vkr::Material> m_roomMaterial;
    std::shared_ptr<vkr::Material> m_grayscaleRoomMaterial;

    std::shared_ptr<tinygltf::Model> m_gltfModel;

    // Objects
    std::shared_ptr<vkr::SceneObject> m_leftRoom;
    std::shared_ptr<vkr::SceneObject> m_rightRoom;
    std::shared_ptr<vkr::SceneObject> m_model;

    std::unique_ptr<vkr::DescriptorPool> m_descriptorPool;

    vkr::Timer m_frameTimer;
    vkr::Timer m_appTime;
    std::uint32_t m_fpsDrawnFrames = 0;
    float m_lastFrameTime = 0.0f;
    float m_lastFenceTime = 0.0f;

    glm::vec3 m_cameraPos = glm::vec3(0.0f, -3.0f, 3.0f);
    glm::vec3 m_cameraRotation = glm::vec3(-45.0f, 0.0f, 0.0f);
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