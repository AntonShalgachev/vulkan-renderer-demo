#include "DemoApplication.h"

#include <memory>
#include <array>
#include <vector>
#include <sstream>
#include <iostream>

#include "tiny_gltf.h"
#include "glm.h"
#include "magic_enum.hpp"

#pragma warning(push, 0)
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#pragma warning(pop)

#include "Application.h"
#include "DescriptorPool.h"
#include "Device.h"
#include "Instance.h" // for imgui
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "PhysicalDevice.h"
#include "Pipeline.h" // imgui
#include "Queue.h" // imgui
#include "Renderer.h"
#include "RenderPass.h" // imgui
#include "Sampler.h"
#include "SceneObject.h"
#include "ScopedOneTimeCommandBuffer.h" // imgui
#include "ShaderModule.h"
#include "Swapchain.h" // imgui
#include "Texture.h"
#include "DebugMessage.h"

#include "DebugConsole.h"
#include "CommandLine.h"

namespace
{
    const uint32_t TARGET_WINDOW_WIDTH = 1900;
    const uint32_t TARGET_WINDOW_HEIGHT = 1000;

#ifdef _DEBUG
    bool const VALIDATION_ENABLED = true;
#else
    bool const VALIDATION_ENABLED = false;
#endif
    bool const API_DUMP_ENABLED = false;

//     const std::string GLTF_MODEL_PATH = "data/models/Duck/glTF/Duck.gltf";
    const std::string GLTF_MODEL_PATH = "data/models/GearboxAssy/glTF/GearboxAssy.gltf";
//     const std::string GLTF_MODEL_PATH = "data/models/ReciprocatingSaw/glTF/ReciprocatingSaw.gltf";

    const glm::vec3 LIGHT_POS = glm::vec3(0.0, 50.0f, 50.0f);
    const glm::vec3 CAMERA_POS = glm::vec3(0.0f, 0.0f, 4.0f);
    const glm::vec3 CAMERA_ANGLES = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::quat createRotation(glm::vec3 const& eulerDegrees)
    {
        glm::vec3 radians = glm::radians(eulerDegrees);
        return glm::toQuat(glm::eulerAngleYXZ(radians.y, radians.x, radians.z));
    }

    glm::mat4 createMatrix(std::vector<double> const& linearMatrix)
    {
        if (linearMatrix.empty())
            return glm::identity<glm::mat4>();
        if (linearMatrix.size() == 16)
            return glm::make_mat4(linearMatrix.data());

        throw std::runtime_error("unexpected linear matrix size");
    }

    glm::vec4 createColor(std::vector<double> const& flatColor)
    {
        if (flatColor.empty())
            return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (flatColor.size() == 4)
            return glm::make_vec4(flatColor.data());

        throw std::runtime_error("unexpected flat color size");
    }

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

//////////////////////////////////////////////////////////////////////////

DemoApplication::DemoApplication()
{
    coil::Bindings& bindings = DebugConsole::instance().bindings();

    bindings["imgui.demo"] = [this]() { m_drawImguiDemo = !m_drawImguiDemo; };
    bindings["imgui.debugger"] = [this]() { m_drawImguiDebugger = !m_drawImguiDebugger; };
    bindings["imgui.styles"] = [this]() { m_drawImguiStyleEditor = !m_drawImguiStyleEditor; };
    bindings["imgui.reload"] = [this]() { m_reloadImgui = true; };

    bindings["camera.znear"] = coil::property(&DemoApplication::getCameraNearZ, &DemoApplication::setCameraNearZ, this);
    bindings["camera.zfar"] = coil::property(&DemoApplication::getCameraFarZ, &DemoApplication::setCameraFarZ, this);
    bindings["camera.speed"] = coil::variable(&m_cameraSpeed);
    bindings["camera.mouse_sensitivity"] = coil::variable(&m_mouseSensitivity);

    bindings["fps"] = [this]() { m_showFps = !m_showFps; };
    bindings["fps.update_period"] = coil::variable(&m_fpsUpdatePeriod);

    m_keyState.resize(1 << 8 * sizeof(char), false);

    m_window = std::make_unique<vkr::Window>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
    m_window->addResizeCallback([this](int, int) { onFramebufferResized(); });
    m_window->addKeyCallback([this](vkr::Window::Action action, vkr::Window::Key key, char c, vkr::Window::Modifiers modifiers) { onKey(action, key, c, modifiers); });
    m_window->addMouseMoveCallback([this](glm::vec2 const& delta) { onMouseMove(delta); });

    auto messageCallback = [](vkr::DebugMessage m)
    {
        assert(false);
    };

    m_application = std::make_unique<vkr::Application>("Vulkan demo", VALIDATION_ENABLED, API_DUMP_ENABLED, *m_window, std::move(messageCallback));

    m_renderer = std::make_unique<vkr::Renderer>(getApp());
    m_renderer->setWaitUntilWindowInForegroundCallback([this]() { m_window->waitUntilInForeground(); });

    loadImgui();

    m_notifications.add("Some notification");
    m_notifications.add("Another notification");
    m_notifications.add("A long long long long long long long long notification");
    m_notifications.add("A rather long notification");

    bindings["window.resize"] = coil::bind(&vkr::Window::resize, m_window.get());
    bindings["window.width"] = coil::bind(&vkr::Window::getWidth, m_window.get());
    bindings["window.height"] = coil::bind(&vkr::Window::getHeight, m_window.get());

    bindings["scene.load"] = coil::bind(&DemoApplication::loadScene, this);

    loadScene(GLTF_MODEL_PATH);
}

DemoApplication::~DemoApplication()
{
    // TODO unregister debug commands

    unloadImgui();

    // move to the renderer
    getApp().getDevice().waitIdle();
}

void DemoApplication::registerCommandLineOptions(CommandLine& commandLine)
{
    commandLine.add("--execute")
        .default_value(std::vector<std::string>{})
        .append()
        .help("execute a given command");
}

void DemoApplication::run()
{
    auto lines = CommandLine::instance().get<std::vector<std::string>>("--execute");
    for (auto const& line : lines)
        DebugConsole::instance().execute(line);

    m_frameTimer.start();
    m_window->startEventLoop([this]() { drawFrame(); });
}

void DemoApplication::loadImgui()
{
    if (ImGui::GetCurrentContext())
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    io.Fonts->AddFontDefault();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_window->getHandle(), true);

    m_imguiDescriptorPool = std::make_unique<vkr::DescriptorPool>(getApp(), 1);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = getApp().getInstance().getHandle();
    init_info.PhysicalDevice = getApp().getPhysicalDevice().getHandle();
    init_info.Device = getApp().getDevice().getHandle();
    init_info.QueueFamily = getApp().getDevice().getGraphicsQueue().getFamily().getIndex();
    init_info.Queue = getApp().getDevice().getGraphicsQueue().getHandle();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_imguiDescriptorPool->getHandle();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = 2; // TODO fetch?
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

void DemoApplication::unloadImgui()
{
    if (!ImGui::GetCurrentContext())
        return;

    getApp().getDevice().waitIdle();

    m_imguiDescriptorPool = nullptr;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DemoApplication::onFramebufferResized()
{
    m_renderer->onFramebufferResized();
    m_application->onSurfaceChanged();
}

void DemoApplication::onKey(vkr::Window::Action action, vkr::Window::Key key, char c, vkr::Window::Modifiers mods)
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

    if (c == '`' && action == vkr::Window::Action::Press)
        m_debugConsole.toggle();
}

void DemoApplication::onMouseMove(glm::vec2 const& delta)
{
    if (!m_activeCameraObject)
        return;

    glm::vec3 angleDelta = m_mouseSensitivity * glm::vec3{ -delta.y, -delta.x, 0.0f };
    glm::quat rotationDelta = createRotation(angleDelta);

    vkr::Transform& cameraTransform = m_activeCameraObject->getTransform();
    cameraTransform.setLocalRotation(cameraTransform.getLocalRotation() * rotationDelta);
}

std::unique_ptr<vkr::SceneObject> DemoApplication::createSceneObject(std::shared_ptr<tinygltf::Model> const& model, tinygltf::Node const& node)
{
    std::unique_ptr<vkr::SceneObject> object = std::make_unique<vkr::SceneObject>();

    object->getTransform().setLocalMatrix(createMatrix(node.matrix));

    if (node.mesh >= 0)
    {
        std::size_t const meshIndex = static_cast<std::size_t>(node.mesh);

        std::size_t const primitiveIndex = 0;

        tinygltf::Mesh const& gltfMesh = model->meshes[meshIndex];
        tinygltf::Primitive const& gltfPrimitive = gltfMesh.primitives[primitiveIndex];

        auto mesh = std::make_shared<vkr::Mesh>(getApp(), model, gltfPrimitive);
        object->setMesh(mesh);

        std::size_t const materialIndex = static_cast<std::size_t>(gltfPrimitive.material);
        tinygltf::Material const& gltfMaterial = model->materials[materialIndex];

        tinygltf::PbrMetallicRoughness const& gltfRoughness = gltfMaterial.pbrMetallicRoughness;

        auto material = std::make_shared<vkr::Material>();
        material->setShaderKey(m_defaultShaderKey);
        material->setColor(createColor(gltfRoughness.baseColorFactor));

        if (gltfRoughness.baseColorTexture.index >= 0)
        {
            // TODO don't create image here, it could be used by several meshes
            std::size_t const imageIndex = static_cast<std::size_t>(gltfRoughness.baseColorTexture.index);
            tinygltf::Image const& gltfImage = model->images[imageIndex];
            auto texture = std::make_shared<vkr::Texture>(getApp(), gltfImage);
            material->setTexture(texture);
            material->setSampler(m_defaultSampler);
        }

        object->setMaterial(material);
    }

    if (node.camera >= 0)
    {
        std::size_t const cameraIndex = static_cast<std::size_t>(node.camera);
        tinygltf::Camera const& gltfCamera = model->cameras[cameraIndex];

        if (gltfCamera.type == "perspective")
        {
            float const aspect = static_cast<float>(gltfCamera.perspective.aspectRatio);
            float const fov = static_cast<float>(gltfCamera.perspective.yfov);
            float const znear = static_cast<float>(gltfCamera.perspective.znear);
            float const zfar = static_cast<float>(gltfCamera.perspective.zfar);

            auto camera = std::make_shared<vkr::Camera>();
            camera->setAspect(aspect);
            camera->setFov(glm::degrees(fov));
            camera->setPlanes(znear, zfar);

            object->setCamera(camera);
        }
    }

    return object;
}

std::shared_ptr<vkr::SceneObject> DemoApplication::createSceneObjectWithChildren(std::shared_ptr<tinygltf::Model> const& model, std::vector<std::shared_ptr<vkr::SceneObject>>& hierarchy, std::size_t nodeIndex)
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

std::vector<std::shared_ptr<vkr::SceneObject>> DemoApplication::createSceneObjectHierarchy(std::shared_ptr<tinygltf::Model> const& model)
{
    std::vector<std::shared_ptr<vkr::SceneObject>> hierarchy;

    std::size_t const sceneIndex = static_cast<std::size_t>(model->defaultScene);
    tinygltf::Scene const& scene = model->scenes[sceneIndex];
    for (std::size_t i = 0; i < scene.nodes.size(); i++)
    {
        std::size_t const nodeIndex = static_cast<std::size_t>(scene.nodes[i]);
        createSceneObjectWithChildren(model, hierarchy, nodeIndex);
    }

    return hierarchy;
}

void DemoApplication::clearScene()
{
    // move to the renderer
    getApp().getDevice().waitIdle();

    m_renderer->clearObjects();
    m_cameraObjects.clear();
    m_activeCameraObject = nullptr;
}

void DemoApplication::loadScene(std::string const& gltfPath)
{
    clearScene();

    m_defaultShaderKey = vkr::Shader::Key{}
        .addStage(vkr::ShaderModule::Type::Vertex, "data/shaders/compiled/default.vert.spv")
        .addStage(vkr::ShaderModule::Type::Fragment, "data/shaders/compiled/default.frag.spv");
    m_defaultSampler = std::make_shared<vkr::Sampler>(getApp());

    auto gltfModel = loadModel(gltfPath);

    auto hierarchy = createSceneObjectHierarchy(gltfModel);
    for (auto const& object : hierarchy)
    {
        m_renderer->addObject(object);

        if (std::shared_ptr<vkr::Camera> const& camera = object->getCamera())
            m_cameraObjects.push_back(object);
    }

    if (!m_cameraObjects.empty())
        m_activeCameraObject = m_cameraObjects.front();

    if (!m_activeCameraObject)
    {
        auto cameraObject = std::make_shared<vkr::SceneObject>();
        cameraObject->setCamera(std::make_shared<vkr::Camera>());

        vkr::Transform& cameraTransform = cameraObject->getTransform();
        cameraTransform.setLocalPos(CAMERA_POS);
        cameraTransform.setLocalRotation(createRotation(CAMERA_ANGLES));

        m_activeCameraObject = cameraObject;
    }

    m_renderer->setCamera(m_activeCameraObject);

    m_light = std::make_shared<vkr::Light>();
    m_light->getTransform().setLocalPos(LIGHT_POS);
    m_renderer->setLight(m_light);
}

void DemoApplication::updateUI(float frameTime, float fenceTime)
{
    if (m_reloadImgui)
    {
        unloadImgui();
        loadImgui();

        m_reloadImgui = false;
    }

    if (!ImGui::GetCurrentContext())
        return;

    ImGuiIO& io = ImGui::GetIO();

    // TODO implement ImGui bindings manually
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_drawImguiDemo)
        ImGui::ShowDemoWindow(&m_drawImguiDemo);
    if (m_drawImguiDebugger)
        ImGui::ShowMetricsWindow(&m_drawImguiDebugger);
    if (m_drawImguiStyleEditor)
        ImGui::ShowStyleEditor();

    float cpuUtilizationRatio = frameTime > 0 ? 1.0f - fenceTime / frameTime : 1.0f;

    {
        const float padding = 10.0f;
        auto viewport = ImGui::GetMainViewport();
        ImVec2 workPos = viewport->WorkPos;
        ImVec2 workSize = viewport->WorkSize;
        ImVec2 windowPos{ workPos.x + workSize.x - padding, workPos.y + padding };
        ImGui::SetNextWindowPos(windowPos, 0, { 1.0f, 0.0f });
    }

    // TODO move to a separate class
    if (m_showFps)
    {
        ImGuiWindowFlags fpsWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Debug", nullptr, fpsWindowFlags);
        ImGui::Text("Frame time %.3f ms", frameTime * 1000.0f);
        ImGui::Text("Fence time %.3f ms", fenceTime * 1000.0f);
        ImGui::Text("CPU Utilization %.2f%%", cpuUtilizationRatio * 100.0f);
        if (ImGui::Button(m_paused ? "Unpause" : "Pause"))
        {
            m_paused = !m_paused;
        }
        static std::vector<std::string> texts = {
            "Lorem ipsum dolor sit amet",
            "consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore",
            "et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation",
        };
        static std::size_t nextIndex = 0;
        if (ImGui::Button("Add notification"))
        {
            m_notifications.add(texts[nextIndex % texts.size()]);
            nextIndex++;
        }
        ImGui::End();
    }

    m_notifications.draw();
    m_debugConsole.draw();

    ImGui::Render();

    m_window->setCanCaptureCursor(!io.WantCaptureMouse);
}

void DemoApplication::drawFrame()
{
    update();
    m_renderer->draw();
    m_fpsDrawnFrames++;
}

void DemoApplication::update()
{
    if (m_frameTimer.getTime() > m_fpsUpdatePeriod)
    {
        float multiplier = 1.0f / static_cast<float>(m_fpsDrawnFrames);
        m_lastFrameTime = m_frameTimer.loop() * multiplier;
        m_lastFenceTime = m_renderer->getCumulativeFenceTime() * multiplier;
        m_renderer->resetCumulativeFenceTime();
        m_fpsDrawnFrames = 0;
    }

    float const dt = m_lastFrameTime;

    m_notifications.update(dt);

    updateUI(m_lastFrameTime, m_lastFenceTime);
    updateScene(dt);
    updateCamera(dt);
}

void DemoApplication::updateScene(float)
{

}

void DemoApplication::updateCamera(float dt)
{
    if (!m_activeCameraObject)
        return;

    // TODO handle input properly
    if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
        return;

    vkr::Transform& cameraTransform = m_activeCameraObject->getTransform();

    glm::vec3 posDelta = glm::zero<glm::vec3>();

    if (m_keyState['A'])
        posDelta += -cameraTransform.getRightVector();
    if (m_keyState['D'])
        posDelta += cameraTransform.getRightVector();
    if (m_keyState['S'])
        posDelta += -cameraTransform.getForwardVector();
    if (m_keyState['W'])
        posDelta += cameraTransform.getForwardVector();
    if (m_keyState['Q'])
        posDelta += -cameraTransform.getUpVector();
    if (m_keyState['E'])
        posDelta += cameraTransform.getUpVector();

    if (glm::length2(posDelta) > glm::epsilon<float>())
        posDelta = glm::normalize(posDelta);

    cameraTransform.setWorldPos(cameraTransform.getWorldPos() + m_cameraSpeed * dt * posDelta);
}

float DemoApplication::getCameraNearZ()
{
    return m_activeCameraObject->getCamera()->nearZ();
}

void DemoApplication::setCameraNearZ(float nearZ)
{
    m_activeCameraObject->getCamera()->setNearZ(nearZ);
}

float DemoApplication::getCameraFarZ()
{
    return m_activeCameraObject->getCamera()->farZ();
}

void DemoApplication::setCameraFarZ(float farZ)
{
    m_activeCameraObject->getCamera()->setFarZ(farZ);
}
