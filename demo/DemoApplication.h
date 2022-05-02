#pragma once

#include <memory>

#include "Shader.h"
#include "Timer.h"
#include "Window.h"

#include "ui/DebugConsoleWidget.h"
#include "ui/NotificationManager.h"

class CommandLine;

namespace vkr
{
    class Application;
    class Renderer;
    class DescriptorPool;
    class Sampler;
    class SceneObject;
    class Light;
}

namespace tinygltf
{
    class Model;
    class Node;
}

struct ICommandLineOptionProvider
{
    virtual void setup(CommandLine& commandLine) const = 0;
};

struct ApplicationOptionProvider : public ICommandLineOptionProvider
{
    void setup(CommandLine& commandLine) const override;
};

class DemoApplication
{
public:
    DemoApplication();

    ~DemoApplication();

    void setupCommandLine(CommandLine& commandLine);

    void run();

private:
    void registerCommandLineOptionProviders();

    template<typename T>
    void registerCommandLineOptionProvider()
    {
        static_assert(std::is_convertible_v<T*, ICommandLineOptionProvider*>, "T should derive from ICommandLineOptionProvider");
        m_commandLineOptionProviders.push_back(std::make_unique<T>());
    }

    void initImGui();

    void onFramebufferResized();

    void onKey(vkr::Window::Action action, vkr::Window::Key key, char c, vkr::Window::Modifiers mods);

    void onMouseMove(glm::vec2 const& delta);

    void createRenderer();

    glm::quat createRotation(glm::vec3 const& eulerDegrees);

    void loadResources();

    // TOOD move to utils or anonymous namespace
    glm::mat4 createMatrix(std::vector<double> const& linearMatrix);

    glm::vec4 createColor(std::vector<double> const& flatColor);

    std::unique_ptr<vkr::SceneObject> createSceneObject(std::shared_ptr<tinygltf::Model> const& model, tinygltf::Node const& node);

    std::shared_ptr<vkr::SceneObject> createSceneObjectWithChildren(std::shared_ptr<tinygltf::Model> const& model, std::vector<std::shared_ptr<vkr::SceneObject>>& hierarchy, std::size_t nodeIndex);

    std::vector<std::shared_ptr<vkr::SceneObject>> createSceneObjectHierarchy(std::shared_ptr<tinygltf::Model> const& model);

    void createSceneObjects();

    void updateUI(float frameTime, float fenceTime);

    void drawFrame();

    void update();

    void updateScene(float);

    void updateCamera(float dt);

    float getCameraNearZ();

    void setCameraNearZ(float nearZ);

    float getCameraFarZ();

    void setCameraFarZ(float farZ);

    vkr::Application const& getApp() { return *m_application; }

private:
    std::vector<std::unique_ptr<ICommandLineOptionProvider>> m_commandLineOptionProviders;

    std::unique_ptr<vkr::Window> m_window;

    std::unique_ptr<vkr::Application> m_application;
    std::unique_ptr<vkr::Renderer> m_renderer;

    std::shared_ptr<vkr::SceneObject> m_activeCameraObject;

    // Resources
    std::shared_ptr<vkr::Sampler> m_defaultSampler;

    vkr::Shader::Key m_defaultShaderKey;

    std::shared_ptr<tinygltf::Model> m_gltfModel;

    // Objects
    std::shared_ptr<vkr::SceneObject> m_model;
    std::vector<std::shared_ptr<vkr::SceneObject>> m_cameraObjects;
    std::shared_ptr<vkr::Light> m_light;

    std::unique_ptr<vkr::DescriptorPool> m_imguiDescriptorPool;

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

    ui::NotificationManager m_notifications;
    ui::DebugConsoleWidget m_debugConsole;

    bool m_drawImguiDemo = false;
    bool m_drawImguiDebugger = false;
    bool m_drawImguiStyleEditor = false;

    bool m_showFps = true;
    float m_fpsUpdatePeriod = 0.2f;
};
