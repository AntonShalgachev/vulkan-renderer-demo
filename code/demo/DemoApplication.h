#pragma once

#include <memory>

#include "Shader.h"
#include "Timer.h"
#include "Window.h"

#include "ui/DebugConsoleWidget.h"
#include "ui/NotificationManager.h"

#include "ScopedDebugCommands.h"

class CommandLine;

namespace vkr
{
    class Application;
    class Renderer;
    class DescriptorPool;
    class Sampler;
    class SceneObject;
    class Light;
    class BufferWithMemory;
}

namespace tinygltf
{
    class Model;
    class Node;
}

struct GltfVkResources
{
    std::vector<std::unique_ptr<vkr::BufferWithMemory>> buffers;
};

class DemoApplication
{
public:
    DemoApplication();
    ~DemoApplication();

    static void registerCommandLineOptions(CommandLine& commandLine);

    void run();

private:
    void loadImgui();
    void unloadImgui();

    void onFramebufferResized();
    void onKey(vkr::Window::Action action, vkr::Window::Key key, char c, vkr::Window::Modifiers mods);
    void onMouseMove(glm::vec2 const& delta);

    std::unique_ptr<vkr::SceneObject> createSceneObject(std::shared_ptr<tinygltf::Model> const& model, tinygltf::Node const& node);
    std::shared_ptr<vkr::SceneObject> createSceneObjectWithChildren(std::shared_ptr<tinygltf::Model> const& model, std::vector<std::shared_ptr<vkr::SceneObject>>& hierarchy, std::size_t nodeIndex);
    std::vector<std::shared_ptr<vkr::SceneObject>> createSceneObjectHierarchy(std::shared_ptr<tinygltf::Model> const& model);

    void clearScene();
    bool loadScene(std::string const& gltfPath);

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
    ScopedDebugCommands m_commands;

    std::unique_ptr<vkr::Window> m_window;

    std::unique_ptr<vkr::Application> m_application;
    std::unique_ptr<vkr::Renderer> m_renderer;

    std::shared_ptr<vkr::SceneObject> m_activeCameraObject;

    std::unique_ptr<GltfVkResources> m_gltfResources;

    // Resources
    std::shared_ptr<vkr::Sampler> m_defaultSampler;

    vkr::Shader::Key m_defaultShaderKey;

    // Objects
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

    bool m_reloadImgui = false;
};
