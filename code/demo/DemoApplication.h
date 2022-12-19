#pragma once

#include "Timer.h"
#include "GlfwWindow.h"

#include "ui/DebugConsoleWidget.h"
#include "ui/NotificationManager.h"

#include "vkgfx/Handles.h"
#include "vkgfx/PipelineKey.h"
#include "vkgfx/TestObject.h"

#include "services/Services.h"

#include "ScopedDebugCommands.h"

#include "nstl/vector.h"
#include "nstl/unordered_map.h"
#include "nstl/optional.h"
#include "nstl/unique_ptr.h"

class CommandLineService;
class ImGuiDrawer;
class ShaderPackage;

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

struct cgltf_data;
struct cgltf_scene;

struct DemoAttributeSemanticsConfiguration
{
    bool hasColor = false;
    bool hasUv = false;
    bool hasNormal = false;
    bool hasTangent = false;
};

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

struct DemoCamera
{
    vkgfx::TestCameraTransform transform;
    std::size_t parametersIndex = 0;
};

struct GltfResources
{
    nstl::vector<vkgfx::BufferHandle> buffers;
    nstl::vector<vkgfx::ImageHandle> images;
    nstl::vector<vkgfx::SamplerHandle> samplers;
    nstl::vector<vkgfx::TextureHandle> textures;
    nstl::vector<DemoMaterial> materials;
    nstl::vector<nstl::vector<DemoMesh>> meshes;

    nstl::unordered_map<nstl::string, vkgfx::ShaderModuleHandle> shaderModules;

    nstl::vector<vkgfx::TestCameraParameters> cameraParameters;

    nstl::vector<vkgfx::BufferHandle> additionalBuffers; // TODO think how to store all created resources better
};

struct DemoScene
{
    nstl::vector<vkgfx::TestObject> objects;
    nstl::vector<DemoCamera> cameras;
};

class DemoApplication
{
public:
    ~DemoApplication();

    static void registerCommandLineOptions(CommandLineService& commandLine);

    bool init(int argc, char** argv);
    void run();

private:
    void init();
    void createResources();

    void loadImgui();
    void unloadImgui();

    void onKey(vkr::GlfwWindow::Action action, vkr::GlfwWindow::Key key, char c, vkr::GlfwWindow::Modifiers mods);
    void onMouseMove(glm::vec2 const& delta);

    DemoScene createDemoScene(cgltf_data const& gltfModel, cgltf_scene const& gltfScene) const;
    void createDemoObjectRecursive(cgltf_data const& gltfModel, std::size_t nodeIndex, glm::mat4 parentTransform, DemoScene& scene) const;

    void clearScene();
    bool loadScene(nstl::string const& gltfPath);
    bool loadGltfModel(nstl::string_view basePath, cgltf_data const& model);

    void updateUI(float frameTime);
    void drawFrame();

    void update();
    void updateScene(float);
    void updateCamera(float dt);

private:
    Services m_services;

    ScopedDebugCommands m_commands{ m_services };

    nstl::unique_ptr<vkr::GlfwWindow> m_window;

    nstl::unique_ptr<ImGuiDrawer> m_imGuiDrawer;

    nstl::unique_ptr<GltfResources> m_gltfResources;

    DemoScene m_demoScene;

    // Resources
    nstl::unique_ptr<ShaderPackage> m_defaultVertexShader;
    nstl::unique_ptr<ShaderPackage> m_defaultFragmentShader;

    vkgfx::SamplerHandle m_defaultSampler;
    vkgfx::ImageHandle m_defaultAlbedoImage;
    vkgfx::TextureHandle m_defaultAlbedoTexture;
    vkgfx::ImageHandle m_defaultNormalMapImage;
    vkgfx::TextureHandle m_defaultNormalMapTexture;

    vkc::Timer m_frameTimer;
    vkc::Timer m_appTime;
    std::uint32_t m_fpsDrawnFrames = 0;
    float m_lastFrameTime = 0.0f;

    glm::vec3 m_cameraRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_mouseSensitivity = 0.3f;
    float m_cameraSpeed = 5.0f;
    bool m_paused = false;

    bool m_validationEnabled = false;

    nstl::vector<bool> m_keyState;
    vkr::GlfwWindow::Modifiers m_modifiers = vkr::GlfwWindow::Modifiers::None;

    nstl::optional<ui::NotificationManager> m_notifications;
    nstl::optional<ui::DebugConsoleWidget> m_debugConsole;

    bool m_drawImguiDemo = false;
    bool m_drawImguiDebugger = false;
    bool m_drawImguiStyleEditor = false;

    bool m_showFps = true;
    float m_fpsUpdatePeriod = 0.2f;

    bool m_reloadImgui = false;

    nstl::string m_currentScenePath = "";

    nstl::unique_ptr<vkgfx::Renderer> m_renderer;

    vkgfx::TestCameraTransform m_cameraTransform;
    vkgfx::TestCameraParameters m_cameraParameters;
    vkgfx::TestLightParameters m_lightParameters;
};
