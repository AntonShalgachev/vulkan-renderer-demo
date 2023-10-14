#pragma once

#include "common/Timer.h"
#include "GlfwWindow.h"

#include "ui/DebugConsoleWidget.h"
#include "ui/NotificationManager.h"
#include "ui/MemoryViewerWindow.h"

#include "vkgfx/Handles.h"
#include "vkgfx/PipelineKey.h"
#include "vkgfx/TestObject.h"

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

namespace gfx
{
    class renderer;
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

    gfx::descriptorgroup_layout_storage newUniformConfig;
    gfx::renderstate_flags newRenderConfig;
};

struct DemoMeshMetadata
{
    vkgfx::VertexConfiguration vertexConfig;
    gfx::vertex_configuration_storage newVertexConfig;
    DemoAttributeSemanticsConfiguration attributeSemanticsConfig;
    size_t materialIndex = 0;
    editor::assets::Uuid materialUuid;
};

struct DemoMaterial
{
    vkgfx::MaterialHandle handle;
    DemoMaterialMetadata metadata;

    gfx::buffer_handle buffer;
    gfx::descriptorgroup_handle descriptorgroup;
};

struct DemoPrimitive
{
    vkgfx::MeshHandle handle;
    DemoMeshMetadata metadata;

    nstl::vector<gfx::buffer_with_offset> vertexBuffers;
    gfx::buffer_with_offset indexBuffer;
    gfx::index_type indexType = gfx::index_type::uint16;
    size_t indexCount = 0;
};

struct DemoMesh
{
    nstl::vector<DemoPrimitive> primitives;
};

struct DemoCamera
{
    vkgfx::TestCameraTransform transform;
    size_t parametersIndex = 0;
};

struct GltfResources
{
    nstl::vector<vkgfx::BufferHandle> buffers;
    nstl::vector<vkgfx::ImageHandle> images;
    nstl::vector<vkgfx::SamplerHandle> samplers;
    nstl::vector<vkgfx::TextureHandle> textures;
    nstl::vector<DemoMaterial> materials;
    nstl::vector<DemoMesh> meshes;

    nstl::unordered_map<nstl::string, vkgfx::ShaderModuleHandle> shaderModules;

    nstl::vector<vkgfx::TestCameraParameters> cameraParameters;

    nstl::vector<vkgfx::BufferHandle> additionalBuffers; // TODO think how to store all created resources better
};

struct EditorGltfResources
{
    nstl::unordered_map<editor::assets::Uuid, vkgfx::ImageHandle> images;
    nstl::unordered_map<editor::assets::Uuid, gfx::image_handle> newImages;
    nstl::unordered_map<editor::assets::Uuid, DemoMaterial> materials;
    nstl::unordered_map<editor::assets::Uuid, DemoMesh> meshes;
    nstl::unordered_map<editor::assets::Uuid, vkgfx::BufferHandle> meshBuffers;
    nstl::unordered_map<editor::assets::Uuid, gfx::buffer_handle> newMeshBuffers;

    nstl::unordered_map<nstl::string, vkgfx::ShaderModuleHandle> shaderModules;
    nstl::unordered_map<nstl::string, gfx::shader_handle> newShaderModules;

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

    DemoScene createDemoScene(cgltf_data const& gltfModel, cgltf_scene const& gltfScene) const;
    void createDemoObjectRecursive(cgltf_data const& gltfModel, size_t nodeIndex, tglm::mat4 parentTransform, DemoScene& scene) const;

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

    void createTestResources();
    void drawTest();

private:
    Services m_services;

    nstl::unique_ptr<gfx::renderer> m_newRenderer;

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

    DemoScene m_demoScene;

    // Resources
    nstl::unique_ptr<ShaderPackage> m_defaultVertexShader;
    nstl::unique_ptr<ShaderPackage> m_newDefaultVertexShader;
    nstl::unique_ptr<ShaderPackage> m_defaultFragmentShader;
    nstl::unique_ptr<ShaderPackage> m_newDefaultFragmentShader;

    nstl::unique_ptr<ShaderPackage> m_shadowmapVertexShader;
    nstl::unique_ptr<ShaderPackage> m_newShadowmapVertexShader;

    vkgfx::SamplerHandle m_defaultSampler;
    vkgfx::ImageHandle m_defaultAlbedoImage;
    vkgfx::TextureHandle m_defaultAlbedoTexture;
    vkgfx::ImageHandle m_defaultNormalMapImage;
    vkgfx::TextureHandle m_defaultNormalMapTexture;

    gfx::sampler_handle m_newDefaultSampler;
    gfx::image_handle m_newDefaultAlbedoImage;
    gfx::image_handle m_newDefaultNormalMapImage;

    nstl::vector<gfx::renderstate_handle> m_renderstates;

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

    nstl::unique_ptr<vkgfx::Renderer> m_renderer;

    vkgfx::TestCameraTransform m_cameraTransform;
    vkgfx::TestCameraParameters m_cameraParameters;
    vkgfx::TestLightParameters m_lightParameters;

    nstl::unique_ptr<editor::assets::AssetDatabase> m_assetDatabase;
};
