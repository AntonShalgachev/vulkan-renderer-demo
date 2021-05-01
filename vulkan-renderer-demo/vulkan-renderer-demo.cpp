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

#pragma warning(push, 0)
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#pragma warning(pop)

#include "DescriptorPool.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Timer.h"

#include <tiny_gltf.h>
#include "Light.h"

#include "glm.h"
#include <vector>
#include <sstream>

#include "magic_enum.hpp"

namespace
{
    const uint32_t TARGET_WINDOW_WIDTH = 1900;
    const uint32_t TARGET_WINDOW_HEIGHT = 1000;

    const float FPS_PERIOD = 0.2f;

#ifdef _DEBUG
    bool const VALIDATION_ENABLED = true;
#else
    bool const VALIDATION_ENABLED = false;
#endif
    bool const API_DUMP_ENABLED = false;

	const std::string GLTF_MODEL_PATH = "data/models/GearboxAssy/glTF/GearboxAssy.gltf";
    //const float GLTF_MODEL_SCALE = 1.0f;
    const float GLTF_MODEL_SCALE = 0.1f;

    const glm::vec3 LIGHT_POS = glm::vec3(-10.0, 0.0f, 0.0f);
    const glm::vec3 CAMERA_POS = glm::vec3(0.0f, -3.0f, 3.0f);
    const glm::vec3 CAMERA_ANGLES = glm::vec3(-45.0f, 0.0f, 0.0f);
    const float CAMERA_SPEED = 1.0f;

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
        m_keyState.resize(1 << 8 * sizeof(char), false);

        m_window = std::make_unique<vkr::Window>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
        m_window->addResizeCallback([this](int, int) { onFramebufferResized(); });
        m_window->addKeyCallback([this](vkr::Window::Action action, vkr::Window::Key key, char c, vkr::Window::Modifiers modifiers) { onKey(action, key, c, modifiers); });
        m_window->addMouseMoveCallback([this](glm::vec2 const& delta) { onMouseMove(delta); });
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
        ImGuiIO& io = ImGui::GetIO();
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

    void onKey(vkr::Window::Action action, vkr::Window::Key key, char c, vkr::Window::Modifiers mods)
    {
        std::stringstream ss;
        auto separator = "";

        for (vkr::Window::Modifiers value : magic_enum::enum_values<vkr::Window::Modifiers>())
        {
            if (mods & value)
            {
                ss << separator << magic_enum::enum_name(value);
                separator = " | ";
            }
        }

        std::cout << magic_enum::enum_name(action) << ' ' << magic_enum::enum_name(key) << ' ' << ss.str() << ": " << "'" << c << "'" << std::endl;

        std::size_t index = static_cast<std::size_t>(c);
        m_keyState[index] = action == vkr::Window::Action::Press;
        m_modifiers = mods;
    }

    void onMouseMove(glm::vec2 const& delta)
    {
        glm::vec3 angleDelta = m_mouseSensitivity * glm::vec3{ -delta.y, 0.0f, -delta.x };
        glm::quat rotationDelta = createRotation(angleDelta);

        vkr::Transform& cameraTransform = m_renderer->getCamera().getTransform();
        cameraTransform.setRotation(cameraTransform.getRotation() * rotationDelta);
    }

    void createRenderer()
    {
        m_renderer = std::make_unique<vkr::Renderer>(getApp());
        m_renderer->setWaitUntilWindowInForegroundCallback([this]() { m_window->waitUntilInForeground(); });

        vkr::Transform& cameraTransform = m_renderer->getCamera().getTransform();
        cameraTransform.setPos(CAMERA_POS);
        m_cameraRotation = CAMERA_ANGLES;
        cameraTransform.setRotation(createRotation(m_cameraRotation));
    }

    glm::quat createRotation(glm::vec3 const& eulerDegrees)
    {
        glm::vec3 radians = glm::radians(eulerDegrees);
        return glm::toQuat(glm::eulerAngleYXZ(radians.y, radians.x, radians.z));
    }

    void loadResources()
    {
        m_defaultShader = vkr::ShaderBuilder()
            .addStage(vkr::ShaderModule::Type::Vertex, "data/shaders/default.vert.spv")
            .addStage(vkr::ShaderModule::Type::Fragment, "data/shaders/default.frag.spv")
            .build(getApp());
        m_noColorShader = vkr::ShaderBuilder()
            .addStage(vkr::ShaderModule::Type::Vertex, "data/shaders/default.vert.spv")
            .addStage(vkr::ShaderModule::Type::Fragment, "data/shaders/no-color.frag.spv")
            .build(getApp());
        m_testGeometryShader = vkr::ShaderBuilder()
            .addStage(vkr::ShaderModule::Type::Vertex, "data/shaders/normaldebug.vert.spv")
            .addStage(vkr::ShaderModule::Type::Geometry, "data/shaders/normaldebug.geom.spv")
            .addStage(vkr::ShaderModule::Type::Fragment, "data/shaders/color.frag.spv")
            .build(getApp());

        m_defaultSampler = std::make_shared<vkr::Sampler>(getApp());
        m_gltfModel = loadModel(GLTF_MODEL_PATH);
	}

    // TOOD move to utils or anonymous namespace
	glm::mat4 createMatrix(std::vector<double> const& linearMatrix)
	{
		if (linearMatrix.empty())
			return glm::identity<glm::mat4>();
		if (linearMatrix.size() == 16)
			return glm::make_mat4(linearMatrix.data());

		throw std::runtime_error("unexpected linear matrix size");
	}

    std::unique_ptr<vkr::SceneObject> createSceneObject(std::shared_ptr<tinygltf::Model> const& model, tinygltf::Node const& node)
    {
		std::unique_ptr<vkr::SceneObject> object = std::make_unique<vkr::SceneObject>();

        object->getTransform().setLocalMatrix(createMatrix(node.matrix));

        if (node.mesh >= 0)
        {
            std::size_t const meshIndex = static_cast<std::size_t>(node.mesh);

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

			// TODO temp geometry shader test
			//material->setShader(m_testGeometryShader);

			object->setMaterial(material);
        }

        return object;
	}

    std::shared_ptr<vkr::SceneObject> createSceneObjectWithChildren(std::shared_ptr<tinygltf::Model> const& model, std::vector<std::shared_ptr<vkr::SceneObject>>& hierarchy, std::size_t nodeIndex)
    {
		tinygltf::Node const& node = model->nodes[nodeIndex];

        std::shared_ptr<vkr::SceneObject> parent = createSceneObject(model, node);
		hierarchy.push_back(parent);
		for (auto childNodeIndex : node.children)
		{
            std::shared_ptr<vkr::SceneObject> child = createSceneObjectWithChildren(model, hierarchy, static_cast<std::size_t>(childNodeIndex));
            parent->getTransform().addChild(child->getTransform());
		}

        return parent;
    }

	std::vector<std::shared_ptr<vkr::SceneObject>> createSceneObjectHierarchy(std::shared_ptr<tinygltf::Model> const& model)
	{
        std::vector<std::shared_ptr<vkr::SceneObject>> hierarchy;

        std::size_t const sceneIndex = static_cast<std::size_t>(model->defaultScene);
        tinygltf::Scene const& scene = model->scenes[sceneIndex];
        for (std::size_t i = 0; i < scene.nodes.size(); i++)
        {
            std::size_t const nodeIndex = static_cast<std::size_t>(scene.nodes[i]);
            std::shared_ptr<vkr::SceneObject> root = createSceneObjectWithChildren(model, hierarchy, nodeIndex);
            root->getTransform().setPos(glm::zero<glm::vec3>());
        }

        return hierarchy;
	}

    void setCamera(vkr::Camera& camera, std::shared_ptr<tinygltf::Model> const& model)
    {
		std::size_t const sceneIndex = static_cast<std::size_t>(model->defaultScene);
		tinygltf::Scene const& scene = model->scenes[sceneIndex];
		for (std::size_t i = 0; i < scene.nodes.size(); i++)
		{
			std::size_t const nodeIndex = static_cast<std::size_t>(scene.nodes[i]);
            tinygltf::Node const& node = model->nodes[nodeIndex];

            if (node.camera < 0)
                continue;

			std::size_t const cameraIndex = static_cast<std::size_t>(node.camera);
            tinygltf::Camera const& cameraParams = model->cameras[cameraIndex];

            if (cameraParams.type != "perspective")
                continue;

            float const aspect = static_cast<float>(cameraParams.perspective.aspectRatio);
            float const fov = static_cast<float>(cameraParams.perspective.yfov);
            float const znear = static_cast<float>(cameraParams.perspective.znear);
            float const zfar = static_cast<float>(cameraParams.perspective.zfar);

            camera.getTransform().setLocalMatrix(createMatrix(node.matrix));
            camera.setAspect(aspect);
            camera.setFov(glm::degrees(fov));
            camera.setPlanes(znear, zfar);

            return;
		}
    }

    void createSceneObjects()
    {
        auto hierarchy = createSceneObjectHierarchy(m_gltfModel);
        for (auto const& object : hierarchy)
            m_renderer->addObject(object);

//         setCamera(m_renderer->getCamera(), m_gltfModel);

        // TODO
//         m_model = hierarchy[0];

//         for (std::uint8_t i = 1; i <= 5; i++)
//         {
// 			std::shared_ptr child = createSceneObjectHierarchy(m_gltfModel);
// 			child->getTransform().setPos({ i * 100.0f, i * 100.0f, 0.0f });
// 			m_model->getTransform().addChild(child->getTransform());
// 			m_renderer->addObject(child);
//         }

        m_light = std::make_shared<vkr::Light>();
        m_light->getTransform().setPos(LIGHT_POS);
        m_renderer->setLight(m_light);
    }

    void updateUI(float frameTime, float fenceTime)
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float cpuUtilizationRatio = 1.0f - fenceTime/frameTime;

        ImGui::Begin("Time", nullptr);
        ImGui::Text("Frame time %.3f ms", frameTime * 1000.0f);
        ImGui::Text("Fence time %.3f ms", fenceTime * 1000.0f);
        ImGui::Text("CPU Utilization %.2f%%", cpuUtilizationRatio * 100.0f);
        if (ImGui::Button(m_paused ? "Unpause" : "Pause"))
        {
            m_paused = !m_paused;
        }
        ImGui::End();

        ImGui::Begin("Camera");
        vkr::Transform& cameraTransform = m_renderer->getCamera().getTransform();
        glm::vec3 cameraPos = cameraTransform.getPos();
        if (ImGui::SliderFloat3("Position", &cameraPos[0], -10.0f, 10.0f, "%.2f", 1.0f))
        {
            cameraTransform.setPos(cameraPos);
        }
        if (ImGui::SliderFloat3("Rotation", &m_cameraRotation[0], -180.0f, 180.0f, "%.1f", 1.0f))
        {
            cameraTransform.setRotation(createRotation(m_cameraRotation));
        }
        ImGui::End();

        ImGui::Begin("Light");
        glm::vec3 lightPos = m_light->getTransform().getPos();
        if (ImGui::SliderFloat3("Position", &lightPos[0], -10.0f, 10.0f, "%.2f", 1.0f))
        {
            m_light->getTransform().setPos(lightPos);
        }
        ImGui::End();

        ImGui::Begin("Input");
        ImGui::SliderFloat("Mouse sensitivity", &m_mouseSensitivity, 0.01f, 1.0f, "%.2f", 1.0f);
        ImGui::SliderFloat("Camera speed", &m_cameraSpeed, 0.1f, 10.0f, "%.2f", 1.0f);
        ImGui::End();

        ImGui::Render();

        m_window->setCanCaptureCursor(!io.WantCaptureMouse);
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
            float multiplier = 1.0f / static_cast<float>(m_fpsDrawnFrames);
            m_lastFrameTime = m_frameTimer.loop() * multiplier;
            m_lastFenceTime = m_renderer->getCumulativeFenceTime() * multiplier;
            m_renderer->resetCumulativeFenceTime();
            m_fpsDrawnFrames = 0;
        }

        float const dt = m_lastFrameTime;

        updateUI(m_lastFrameTime, m_lastFenceTime);
		updateScene(dt);
		updateCamera(dt);
    }

    void updateScene(float)
    {
        if (m_paused)
            return;

        auto updateObject = [](vkr::Transform& transform, float time, bool fixRotation)
        {
            glm::quat initialRotation = glm::identity<glm::quat>();
            if (fixRotation)
                initialRotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            transform.setRotation(glm::angleAxis(time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * initialRotation);
        };

        float time = m_appTime.getTime();

        (void*)&time;

//         m_model->getTransform().setRotation(glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
//         updateObject(m_model->getTransform(), time * 0.5f, true);

//         m_model->getTransform().setScale(glm::vec3(GLTF_MODEL_SCALE));
    }

    void updateCamera(float dt)
    {
        glm::vec3 localDirection = glm::zero<glm::vec3>();

        if (m_keyState['A'])
            localDirection.x -= 1.0f;
        if (m_keyState['D'])
            localDirection.x += 1.0f;
        if (m_keyState['S'])
            localDirection.y -= 1.0f;
        if (m_keyState['W'])
            localDirection.y += 1.0f;
        if (m_keyState['Q'])
            localDirection.z -= 1.0f;
        if (m_keyState['E'])
            localDirection.z += 1.0f;

        vkr::Transform& cameraTransform = m_renderer->getCamera().getTransform();

        glm::vec3 worldDirection = cameraTransform.transformVectorLocalToWorld(localDirection);
        cameraTransform.setPos(cameraTransform.getPos() + m_cameraSpeed * dt * worldDirection);
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
    std::shared_ptr<vkr::Shader> m_testGeometryShader;

    std::shared_ptr<tinygltf::Model> m_gltfModel;

    // Objects
    std::shared_ptr<vkr::SceneObject> m_model;
    std::shared_ptr<vkr::Light> m_light;

    std::unique_ptr<vkr::DescriptorPool> m_descriptorPool;

    vkr::Timer m_frameTimer;
    vkr::Timer m_appTime;
    std::uint32_t m_fpsDrawnFrames = 0;
    float m_lastFrameTime = 0.0f;
    float m_lastFenceTime = 0.0f;

    glm::vec3 m_cameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_mouseSensitivity = 0.3f;
    float m_cameraSpeed = 5.0f;
    bool m_paused = false;

    std::vector<bool> m_keyState;
    vkr::Window::Modifiers m_modifiers = vkr::Window::Modifiers::None;
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