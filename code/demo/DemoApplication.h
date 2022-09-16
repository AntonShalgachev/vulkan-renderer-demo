#pragma once

#include <memory>

#include "Shader.h"
#include "Timer.h"
#include "GlfwWindow.h"

#include "ui/DebugConsoleWidget.h"
#include "ui/NotificationManager.h"

#include "vkgfx/Handles.h"
#include "vkgfx/PipelineKey.h"
#include "vkgfx/TestObject.h"

#include "services/Services.h"

#include "ScopedDebugCommands.h"

class CommandLineService;

namespace vko
{
    class DescriptorPool;
    class Sampler;
}

namespace vkgfx
{
    class Renderer;
    class ResourceManager;

    struct BufferHandle;
    struct ImageHandle;
    struct SamplerHandle;
    struct TextureHandle;
    struct MaterialHandle;
    struct MeshHandle;

    struct TestObject;
}

namespace vkr
{
    class Application;
    class Renderer;
    class SceneObject;
    class Light;
    class BufferWithMemory;
    class ShaderPackage;
    class Texture;
}

namespace tinygltf
{
    class Model;
    class Node;
    class Scene;
}

struct GltfVkResources
{
    std::vector<std::unique_ptr<vkr::BufferWithMemory>> buffers;
    std::vector<std::shared_ptr<vkr::Texture>> textures;
};

// #include "vkgfx/Mesh.h"

// namespace vkgfx
// {
//     struct DescriptorSetLayoutConfiguration
//     {
//         UniformConfiguration uniformConfig;
//     };
// 
//     struct PipelineLayoutConfiguration
//     {
//         // set layouts
//         // push constant ranges
//     };
// 
//     struct PipelineConfiguration
//     {
//         // pipeline layout
//         // shader modules
//         VertexConfiguration vertexConfig;
//         RenderConfiguration renderConfig;
//     };
// }

struct DemoAttributeSemanticsConfiguration
{
    bool hasColor = false;
    bool hasUv = false;
    bool hasNormal = false;
    bool hasTangent = false;
};

// struct DemoShaderConfiguration
// {
//     DemoAttributeSemanticsConfiguration attributeConfig;
//     vkgfx::UniformConfiguration uniformConfig;
// };

struct DemoMaterialMetadata
{
    vkgfx::UniformConfiguration uniformConfig;
    vkgfx::RenderConfiguration renderConfig;
};

struct DemoMeshMetadata
{
    vkgfx::VertexConfiguration vertexConfig;
    DemoAttributeSemanticsConfiguration attributeSemanticsConfig;
    std::size_t materialIndex = 0;
};

struct DemoMaterial
{
    vkgfx::MaterialHandle handle;
    DemoMaterialMetadata metadata;
};

struct DemoMesh
{
    vkgfx::MeshHandle handle;
    DemoMeshMetadata metadata;
};

struct GfxResources
{
    std::vector<vkgfx::BufferHandle> buffers;
    std::vector<vkgfx::ImageHandle> images;
    std::vector<vkgfx::SamplerHandle> samplers;
    std::vector<vkgfx::TextureHandle> textures;
    std::vector<DemoMaterial> materials;
    std::vector<std::vector<DemoMesh>> meshes;

    std::unordered_map<std::string, vkgfx::ShaderModuleHandle> shaderModules;
};

// TODO move to a separate file
struct Scene
{
	std::vector<std::shared_ptr<vkr::SceneObject>> objects;
};

struct DemoScene
{
    std::vector<vkgfx::TestObject> objects;
};

class DemoApplication
{
public:
    DemoApplication();
    ~DemoApplication();

    static void registerCommandLineOptions(CommandLineService& commandLine);

    bool init(int argc, char** argv);
    void run();

private:
    void createServices();
    void destroyServices();

    void loadImgui();
    void unloadImgui();

    void onFramebufferResized();
    void onKey(vkr::GlfwWindow::Action action, vkr::GlfwWindow::Key key, char c, vkr::GlfwWindow::Modifiers mods);
    void onMouseMove(glm::vec2 const& delta);

    std::shared_ptr<vkr::SceneObject> addSceneObjectsFromNode(std::shared_ptr<tinygltf::Model> const& model, tinygltf::Node const& node, Scene& scene);
    std::shared_ptr<vkr::SceneObject> createSceneObjectWithChildren(std::shared_ptr<tinygltf::Model> const& model, Scene& scene, std::size_t nodeIndex);
    Scene createSceneObjectHierarchy(std::shared_ptr<tinygltf::Model> const& model);

    DemoScene createDemoScene(tinygltf::Model const& gltfModel, tinygltf::Scene const& gltfScene) const;
    void createDemoObjectRecursive(tinygltf::Model const& gltfModel, std::size_t nodeIndex, DemoScene& scene) const;

    void clearScene();
    bool loadScene(std::string const& gltfPath);

    void updateUI(float frameTime, float fenceTime);
    void drawFrame();
    void update();
    void updateScene(float);
    void updateCamera(float dt);

    glm::vec3 getCameraPos() const;
    void setCameraPos(glm::vec3 const& pos);
    glm::quat getCameraRotation() const;
    void setCameraRotation(glm::quat const& rotation);

    float getCameraNearZ() const;
    void setCameraNearZ(float nearZ);
    float getCameraFarZ() const;
    void setCameraFarZ(float farZ);

    vkr::Application const& getApp() { return *m_application; }

private:
    Services m_services;

    ScopedDebugCommands m_commands{ m_services };

    std::unique_ptr<vkr::GlfwWindow> m_window;

    std::unique_ptr<vkr::Application> m_application;
    std::unique_ptr<vkr::Renderer> m_renderer;

    std::unique_ptr<GltfVkResources> m_gltfResources;
    std::unique_ptr<GfxResources> m_gfxResources;

    // Resources
    std::unique_ptr<vkr::ShaderPackage> m_defaultVertexShader;
    std::unique_ptr<vkr::ShaderPackage> m_defaultFragmentShader;

    std::shared_ptr<vko::Sampler> m_fallbackSampler;
    std::shared_ptr<vkr::Texture> m_fallbackAlbedo;
    std::shared_ptr<vkr::Texture> m_fallbackNormalMap;

    vkgfx::SamplerHandle m_defaultSampler;
    vkgfx::ImageHandle m_defaultAlbedoImage;
    vkgfx::TextureHandle m_defaultAlbedoTexture;
    vkgfx::ImageHandle m_defaultNormalMapImage;
    vkgfx::TextureHandle m_defaultNormalMapTexture;

    // Objects
    Scene m_scene;
    std::shared_ptr<vkr::SceneObject> m_activeCameraObject;
    std::shared_ptr<vkr::Light> m_light;

    std::unique_ptr<vko::DescriptorPool> m_imguiDescriptorPool;

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
    vkr::GlfwWindow::Modifiers m_modifiers = vkr::GlfwWindow::Modifiers::None;

    std::optional<ui::NotificationManager> m_notifications;
    std::optional<ui::DebugConsoleWidget> m_debugConsole;

    bool m_drawImguiDemo = false;
    bool m_drawImguiDebugger = false;
    bool m_drawImguiStyleEditor = false;

    bool m_showFps = true;
    float m_fpsUpdatePeriod = 0.2f;

    bool m_reloadImgui = false;

    std::string m_currentScenePath = "";

    std::unique_ptr<vkgfx::Renderer> m_newRenderer;
    std::unique_ptr<vkgfx::ResourceManager> m_ownResourceManager;

    vkgfx::ResourceManager* m_resourceManager = nullptr;

    vkgfx::TestCameraTransform m_cameraTransform;
};
