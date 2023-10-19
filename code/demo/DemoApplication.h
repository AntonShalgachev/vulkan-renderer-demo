#pragma once

#include "common/Timer.h"
#include "GlfwWindow.h"
#include "DemoSceneDrawer.h"

#include "ui/DebugConsoleWidget.h"
#include "ui/NotificationManager.h"
#include "ui/MemoryViewerWindow.h"

#include "services/Services.h"

#include "ScopedDebugCommands.h"

#include "editor/assets/Uuid.h"

#include "gfx/resources.h"

#include "nstl/vector.h"
#include "nstl/unordered_map.h"
#include "nstl/optional.h"
#include "nstl/unique_ptr.h"

class CommandLineService;
class ImGuiPlatform;
class ImGuiDrawer;
class ShaderPackage;

namespace editor::assets
{
    class AssetDatabase;
    struct Uuid;
}

namespace gfx
{
    class renderer;
}

struct cgltf_data;
struct cgltf_scene;

struct GltfResources
{
    nstl::vector<DemoTexture*> demoTextures;
    nstl::vector<DemoMaterial*> demoMaterials;
    nstl::vector<DemoMesh*> demoMeshes;
};

struct EditorGltfResources
{
    nstl::unordered_map<editor::assets::Uuid, DemoTexture*> demoTextures;
    nstl::unordered_map<editor::assets::Uuid, DemoMaterial*> demoMaterials;
    nstl::unordered_map<editor::assets::Uuid, DemoMesh*> demoMeshes;
};

struct DemoCameraTransform
{
    tglm::vec3 position = { 0.0f, 0.0f, 0.0f };
    tglm::quat rotation = tglm::quat::identity();
};

struct DemoCameraParameters
{
    float fov = 45.0f;
    float nearZ = 0.1f;
    float farZ = 10000.0f;
};

struct DemoLightParameters
{
    tglm::vec3 position = { 0.0f, 0.0f, 0.0f };
    tglm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    tglm::quat rotation = tglm::quat::identity();
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
    void init();
    void createResources();

    void loadImgui();
    void unloadImgui();

    void onKey(GlfwWindow::Action action, GlfwWindow::OldKey key, char c, GlfwWindow::Modifiers mods);
    void onMouseMove(tglm::vec2 const& delta);

    void createDemoScene(cgltf_data const& gltfModel, cgltf_scene const& gltfScene) const;
    void createDemoObjectRecursive(cgltf_data const& gltfModel, size_t nodeIndex, tglm::mat4 parentTransform) const;

    void clearScene();
    bool loadScene(nstl::string_view gltfPath);
    bool loadGltfModel(nstl::string_view basePath, cgltf_data const& model);

    bool editorLoadScene(editor::assets::Uuid id);
    void editorLoadImage(editor::assets::Uuid id);
    void editorLoadMaterial(editor::assets::Uuid id);
    void editorLoadMesh(editor::assets::Uuid id);

    void updateUI(float frameTime);
    void drawFrame();

    void update();
    void updateScene(float);
    void updateCamera(float dt);

    void draw();

private:
    Services m_services;

    nstl::unique_ptr<gfx::renderer> m_renderer;

    gfx::sampler_handle m_defaultSampler;

    gfx::buffer_handle m_viewProjectionData;
    gfx::buffer_handle m_lightData;
    gfx::descriptorgroup_handle m_cameraDescriptorGroup;

    gfx::buffer_handle m_shadowmapViewProjectionData;
    gfx::descriptorgroup_handle m_shadowmapCameraDescriptorGroup;

    gfx::renderpass_handle m_shadowRenderpass;
    gfx::image_handle m_shadowImage;
    gfx::framebuffer_handle m_shadowFramebuffer;

    ScopedDebugCommands m_commands{ m_services };

    nstl::unique_ptr<GlfwWindow> m_window;

    nstl::unique_ptr<ImGuiPlatform> m_imGuiPlatform;
    nstl::unique_ptr<ImGuiDrawer> m_imGuiDrawer;

    nstl::unique_ptr<GltfResources> m_gltfResources;
    nstl::unique_ptr<EditorGltfResources> m_editorGltfResources;

    nstl::unique_ptr<DemoSceneDrawer> m_sceneDrawer;

    vkc::Timer m_frameTimer;
    vkc::Timer m_appTime;
    uint32_t m_fpsDrawnFrames = 0;
    float m_lastFrameTime = 0.0f;
    float m_time = 0.0f;

    tglm::vec3 m_cameraRotation = { 0.0f, 0.0f, 0.0f };
    float m_mouseSensitivity = 0.3f;
    float m_cameraSpeed = 5.0f;
    bool m_paused = false;

    bool m_validationEnabled = false;

    nstl::vector<bool> m_keyState;
    GlfwWindow::Modifiers m_modifiers = GlfwWindow::Modifiers::None;

    nstl::optional<ui::NotificationManager> m_notifications;
    nstl::optional<ui::DebugConsoleWidget> m_debugConsole;
    nstl::optional<ui::MemoryViewerWindow> m_memoryViewer;

    bool m_drawImguiDemo = false;
    bool m_drawImguiDebugger = false;
    bool m_drawImguiStyleEditor = false;

    bool m_showFps = true;
    float m_fpsUpdatePeriod = 0.2f;

    bool m_reloadImgui = false;

    nstl::string m_currentScenePath = "";

    DemoCameraTransform m_cameraTransform;
    DemoCameraParameters m_cameraParameters;
    DemoLightParameters m_lightParameters;

    nstl::unique_ptr<editor::assets::AssetDatabase> m_assetDatabase;
};
