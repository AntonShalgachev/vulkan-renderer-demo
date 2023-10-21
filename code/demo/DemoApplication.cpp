#include "DemoApplication.h"

#include "cgltf.h"

#include "tglm/tglm.h" // TODO only include what is needed

#include "imgui.h"

#include "ImGuiPlatform.h"
#include "ImGuiDrawer.h"

#include "gfx/renderer.h"

#include "gfx_vk/backend.h"

#include "platform_win64/window.h"

#include "gfx_vk_win64/surface_factory.h"

#include "services/DebugConsoleService.h"
#include "services/CommandLineService.h"
#include "services/DebugDrawService.h"

#include "demo/console/GlmSerializer.h"
#include "demo/console/EnumSerializer.h"
#include "demo/console/EditorSerializer.h"

#include "editor/assets/AssetDatabase.h"
#include "editor/assets/AssetData.h"

#include "common/Utils.h"
#include "common/tiny_ctti.h"

#include "logging/logging.h"

#include "memory/tracking.h"
#include "memory/memory.h"

#include "fs/file.h"

#include "nstl/array.h"
#include "nstl/span.h"
#include "nstl/optional.h"
#include "nstl/string_builder.h"
#include "nstl/scope_exit.h"

#include <math.h>

namespace
{
    const int TARGET_WINDOW_WIDTH = 1900;
    const int TARGET_WINDOW_HEIGHT = 1000;

    constexpr uint32_t SHADOWMAP_RESOLUTION = 1024;
    constexpr uint32_t SHADOWMAP_FOV = 90;

#ifdef _DEBUG
    bool const VALIDATION_ENABLED = true;
#else
    bool const VALIDATION_ENABLED = false;
#endif

    const tglm::vec3 LIGHT_POS = tglm::vec3(0.0, 2.0f, 0.0f);
    const tglm::vec3 LIGHT_COLOR = tglm::vec3(1.0, 1.0f, 1.0f);
    const float LIGHT_INTENSITY = 30.0f;
    const tglm::vec3 CAMERA_POS = tglm::vec3(0.0f, 0.0f, 4.0f);
    const tglm::vec3 CAMERA_ANGLES = tglm::vec3(0.0f, 0.0f, 0.0f);

    struct ShaderViewProjectionData
    {
        tglm::mat4 view;
        tglm::mat4 projection;
    };

    struct ShaderLightData
    {
        tglm::mat4 lightViewProjection;
        alignas(16) tglm::vec3 lightPosition;
        alignas(16) tglm::vec3 lightColor;
    };

    // TODO is there a better way?
    template<typename T>
    size_t findIndex(T const* object, T const* first, [[maybe_unused]] size_t count)
    {
        assert(object >= first);
        size_t index = static_cast<size_t>(object - first);
        assert(index < count);
        return static_cast<size_t>(index);
    }

    tglm::quat createRotation(tglm::vec3 const& eulerDegrees)
    {
        return tglm::quat::from_euler_zyx(tglm::radians(eulerDegrees));
    }

    tglm::mat4 createMatrix(cgltf_node const& node)
    {
        if (node.has_matrix)
            return tglm::mat4(node.matrix);

        tglm::mat4 matrix = tglm::mat4::identity();

        if (node.has_translation)
            tglm::translate(matrix, tglm::vec3{ node.translation });

        if (node.has_rotation)
            tglm::rotate(matrix, tglm::quat{ node.rotation });

        if (node.has_scale)
            tglm::scale(matrix, tglm::vec3{ node.scale });

        return matrix;
    }

    tglm::mat4 createMatrix(editor::assets::ObjectDescription const& object)
    {
        tglm::mat4 matrix = tglm::mat4::identity();

        if (!object.transform)
            return matrix;

        tglm::translate(matrix, object.transform->position);
        tglm::rotate(matrix, object.transform->rotation);
        tglm::scale(matrix, object.transform->scale);

        return matrix;
    }

    tglm::vec4 createColor(nstl::span<float const> flatColor)
    {
        assert(flatColor.size() == 4);
        return tglm::vec4{ flatColor.data(), flatColor.size() };
    }

    gfx::attribute_type findAttributeType(cgltf_type gltfAttributeType, cgltf_component_type gltfComponentType)
    {
        if (gltfAttributeType == cgltf_type_vec2 && gltfComponentType == cgltf_component_type_r_32f)
            return gfx::attribute_type::vec2f;
        if (gltfAttributeType == cgltf_type_vec3 && gltfComponentType == cgltf_component_type_r_32f)
            return gfx::attribute_type::vec3f;
        if (gltfAttributeType == cgltf_type_vec4 && gltfComponentType == cgltf_component_type_r_32f)
            return gfx::attribute_type::vec4f;

        assert(false);
        return gfx::attribute_type::vec4f;
    }

    gfx::attribute_type newFindAttributeType(editor::assets::DataType dataType, editor::assets::DataComponentType componentType)
    {
        if (dataType == editor::assets::DataType::Vec2 && componentType == editor::assets::DataComponentType::Float)
            return gfx::attribute_type::vec2f;
        if (dataType == editor::assets::DataType::Vec3 && componentType == editor::assets::DataComponentType::Float)
            return gfx::attribute_type::vec3f;
        if (dataType == editor::assets::DataType::Vec4 && componentType == editor::assets::DataComponentType::Float)
            return gfx::attribute_type::vec4f;

        assert(false);
        return gfx::attribute_type::vec4f;
    }

    size_t getAttributeByteSize(gfx::attribute_type type)
    {
        size_t gltfFloatSize = 4;

        switch (type)
        {
        case gfx::attribute_type::vec2f:
            return 2 * gltfFloatSize;
        case gfx::attribute_type::vec3f:
            return 3 * gltfFloatSize;
        case gfx::attribute_type::vec4f:
            return 4 * gltfFloatSize;
        case gfx::attribute_type::uint32:
            return 4;
        }

        assert(false);
        return 0;
    }

    nstl::optional<size_t> findAttributeLocation(nstl::string_view name)
    {
        static nstl::vector<nstl::string_view> const attributeNames = { "POSITION", "COLOR_0", "TEXCOORD_0", "NORMAL", "TANGENT" }; // TODO move to the shader metadata

        auto it = attributeNames.find(name);

        if (it != attributeNames.end())
        {
            auto offset = it - attributeNames.begin();
            assert(offset >= 0);
            return static_cast<size_t>(offset);
        }

        return {};
    }

    nstl::optional<size_t> findAttributeLocation(editor::assets::AttributeSemantic semantic)
    {
        using editor::assets::AttributeSemantic;

        static nstl::vector<AttributeSemantic> const attributeSemantics = { AttributeSemantic::Position, AttributeSemantic::Color, AttributeSemantic::Texcoord, AttributeSemantic::Normal, AttributeSemantic::Tangent }; // TODO move to the shader metadata

        auto it = attributeSemantics.find(semantic);

        if (it != attributeSemantics.end())
        {
            auto offset = it - attributeSemantics.begin();
            assert(offset >= 0);
            return static_cast<size_t>(offset);
        }

        return {};
    }

    // TODO move somewhere
    auto toggle(bool* var)
    {
        auto toggler = [var]()
        {
            *var = !*var;
            return *var;
        };
        auto setter = [var](bool val)
        {
            *var = nstl::move(val);
            return *var;
        };

        return coil::overloaded(nstl::move(toggler), nstl::move(setter));
    }
}

//////////////////////////////////////////////////////////////////////////

DemoApplication::DemoApplication() = default;

DemoApplication::~DemoApplication()
{
    clearScene();
    unloadImgui();

    // TODO come up with a better way to destroy objects with captured services
    m_memoryViewer = {};
    m_debugConsole = {};
    m_notifications = {};
    m_commands.clear();

    // TODO remove forced services destruction order
    m_services.setDebugDraw(nullptr);
    m_services.setCommandLine(nullptr);
    m_services.setDebugConsole(nullptr);

    m_services = Services{};
}

void DemoApplication::registerCommandLineOptions(CommandLineService&)
{
    // TODO implement
//     commandLine.add("--execute")
//         .default_value(std::vector<std::string>{})
//         .append()
//         .help("execute a given command");
}

bool DemoApplication::init(int argc, char** argv)
{
    m_services.setDebugConsole(nstl::make_unique<DebugConsoleService>(m_services));
    m_services.setCommandLine(nstl::make_unique<CommandLineService>(m_services));

    auto& commandLine = m_services.commandLine();
    DemoApplication::registerCommandLineOptions(commandLine); // TODO move somewhere to allow others to register custom options

//     logging::info("Current directory: {}", std::filesystem::current_path());

//     if (!std::filesystem::exists("data"))
//         logging::warn("Current directory doesn't contain 'data', probably wrong directory");

    commandLine.add(argc, argv);

    {
        nstl::string contents;
        fs::file f{ "data/cmdline.ini", fs::open_mode::read};
        contents.resize(f.size());
        f.read(contents.data(), contents.size());

        for (nstl::string_view line : vkc::utils::split(contents))
            commandLine.addLine(line);
    }

    if (!commandLine.parse())
    {
        logging::error("Failed to parse command line arguments");
        return false;
    }

    {
        nstl::string args;
        for (nstl::string const& argument : commandLine.getAll())
            args += "'" + argument + "' ";

        logging::info("Command line arguments: {}", args);
    }

    m_notifications = ui::NotificationManager{ m_services };
    m_debugConsole = ui::DebugConsoleWidget{ m_services };
    m_memoryViewer = ui::MemoryViewerWindow{ m_services };

    m_assetDatabase = nstl::make_unique<editor::assets::AssetDatabase>();

    init();

    return true;
}

void DemoApplication::run()
{
    auto const& lines = m_services.commandLine().get("--exec-before-run");
    for (auto const& line : lines)
    {
        [[maybe_unused]] bool success = m_services.debugConsole().execute(line);
        assert(success);
    }

    m_frameTimer.start();
    m_window->startEventLoop([this]() { drawFrame(); });
}

void DemoApplication::init()
{
    m_cameraTransform = {
        .position = CAMERA_POS,
        .rotation = createRotation(CAMERA_ANGLES),
    };

    m_cameraParameters = {
        .fov = 45.0f,
        .nearZ = 0.1f,
        .farZ = 10000.0f,
    };

    m_lightParameters = {
        .position = LIGHT_POS,
        .color = LIGHT_COLOR,
        .intensity = LIGHT_INTENSITY,
    };

    m_validationEnabled = VALIDATION_ENABLED;

    m_commands["imgui.demo"] = ::toggle(&m_drawImguiDemo);
    m_commands["imgui.debugger"] = ::toggle(&m_drawImguiDebugger);
    m_commands["imgui.styles"] = ::toggle(&m_drawImguiStyleEditor);
    m_commands["imgui.reload"] = [this]() { m_reloadImgui = true; };

    m_commands["camera.fov"] = coil::variable(&m_cameraParameters.fov);
    m_commands["camera.znear"] = coil::variable(&m_cameraParameters.nearZ);
    m_commands["camera.zfar"] = coil::variable(&m_cameraParameters.farZ);
    m_commands["camera.speed"] = coil::variable(&m_cameraSpeed);
    m_commands["camera.mouse_sensitivity"] = coil::variable(&m_mouseSensitivity);
    m_commands["camera.pos"] = coil::variable(&m_cameraTransform.position);
    m_commands["camera.angles"] = coil::property([this]() {
        return tglm::degrees(m_cameraTransform.rotation.to_euler_xyz());
    }, [this](tglm::vec3 const& angles) {
        m_cameraTransform.rotation = createRotation(angles);
    });

    m_commands["light.pos"] = coil::variable(&m_lightParameters.position);
    m_commands["light.color"] = coil::variable(&m_lightParameters.color);
    m_commands["light.intensity"] = coil::variable(&m_lightParameters.intensity);

    m_commands["fps"].description("Show/hide the FPS widget") = ::toggle(&m_showFps);
    m_commands["fps.update_period"].description("Update period of the FPS widget") = coil::variable(&m_fpsUpdatePeriod);

    m_commands["vulkan.enable-validation"].description("Enable Vulkan validation layers") = coil::variable(&m_validationEnabled);

    m_commands["window.memory-viewer"].description("Memory viewer window") = [this]() { m_memoryViewer->toggle(); };

    m_commands["assets.import"] = [this](nstl::string_view path) { m_assetDatabase->importAsset(path); };

    m_commands["scene.load-editor"].description("Load scene from the asset").arguments("id") = [this](coil::Context context, editor::assets::Uuid id) {
        if (!editorLoadScene(id))
            context.reportError("Failed to load the scene '" + coil::fromNstlStringView(id.toString()) + "'");
    };

    // TODO find a proper place for these commands
    auto const& lines = m_services.commandLine().get("--exec-before-init");
    for (auto const& line : lines)
    {
        [[maybe_unused]] bool success = m_services.debugConsole().execute(line);
        assert(success);
    }

    m_keyState.resize(1 << 8 * sizeof(char), false);

    m_window = nstl::make_unique<GlfwWindow>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
    m_window->addOldKeyCallback([this](GlfwWindow::Action action, GlfwWindow::OldKey key, char c, GlfwWindow::Modifiers modifiers) { onKey(action, key, c, modifiers); });
    m_window->addOldMouseDeltaCallback([this](float deltaX, float deltaY) { onMouseMove({ deltaX, deltaY }); });

    gfx_vk_win64::surface_factory surfaceFactory{ m_window->getWindowHandle() };

    gfx_vk::config config = {
        .name = "Vulkan Demo",
        .enable_validation = m_validationEnabled,

        .descriptors = {
            .max_sets_per_pool = 2048 * 16,
            .max_descriptors_per_type_per_pool = 4 * 2048 * 16,
        },
    };
    auto backend = nstl::make_unique<gfx_vk::backend>(surfaceFactory, m_window->getFramebufferSize(), config);
    m_renderer = nstl::make_unique<gfx::renderer>(nstl::move(backend));

    m_window->addFramebufferResizeCallback([this](int width, int height) {
        if (width <= 0 || height <= 0)
            return;

        m_renderer->resize_main_framebuffer({ width, height });
    });

    m_services.setDebugDraw(nstl::make_unique<DebugDrawService>(*m_renderer));

    loadImgui();

    m_commands["window.resize"].arguments("width", "height") = coil::bind(&GlfwWindow::resize, m_window.get());
    m_commands["window.width"] = coil::bindProperty(&GlfwWindow::getWindowWidth, m_window.get());
    m_commands["window.height"] = coil::bindProperty(&GlfwWindow::getWindowHeight, m_window.get());

    m_commands["scene.load"].description("Load scene from a GLTF model").arguments("path") = [this](coil::Context context, nstl::string_view path) {
        if (!loadScene(path))
            context.reportError("Failed to load the scene '" + coil::fromNstlStringView(path) + "'");
    };
    m_commands["scene.reload"] = [this]() { loadScene(m_currentScenePath); };
    m_commands["scene.unload"] = coil::bind(&DemoApplication::clearScene, this);

    createResources();

    m_sceneDrawer = nstl::make_unique<DemoSceneDrawer>(*m_renderer, m_shadowRenderpass);
}

void DemoApplication::createResources()
{
    m_defaultSampler = m_renderer->create_sampler({});

    m_viewProjectionData = m_renderer->create_buffer({
        .size = sizeof(ShaderViewProjectionData),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = true,
    });

    m_lightData = m_renderer->create_buffer({
        .size = sizeof(ShaderLightData),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = true,
    });

    m_shadowmapViewProjectionData = m_renderer->create_buffer({
        .size = sizeof(ShaderViewProjectionData),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = true,
    });

    m_shadowmapCameraDescriptorGroup = m_renderer->create_descriptorgroup({
        .entries = nstl::array{
            gfx::descriptorgroup_entry{0, {m_shadowmapViewProjectionData, gfx::descriptor_type::uniform_buffer}},
        }
    });

    m_shadowRenderpass = m_renderer->create_renderpass({
        .color_attachment_formats = {},
        .depth_stencil_attachment_format = gfx::image_format::d32_float,

        .has_presentable_images = false,
        .keep_depth_values_after_renderpass = true,
    });

    m_shadowImage = m_renderer->create_image({
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION,
        .format = gfx::image_format::d32_float,
        .type = gfx::image_type::depth,
        .usage = gfx::image_usage::depth_sampled,
    });

    m_shadowFramebuffer = m_renderer->create_framebuffer({
        .attachments = nstl::array{ m_shadowImage },
        .renderpass = m_shadowRenderpass,
    });

    m_cameraDescriptorGroup = m_renderer->create_descriptorgroup({
        .entries = nstl::array{
            gfx::descriptorgroup_entry{0, {m_viewProjectionData, gfx::descriptor_type::uniform_buffer}},
            gfx::descriptorgroup_entry{1, {m_lightData, gfx::descriptor_type::uniform_buffer}},
            gfx::descriptorgroup_entry{2, {m_shadowImage, m_defaultSampler}},
        }
    });
}

void DemoApplication::loadImgui()
{
    static auto scopeId = memory::tracking::create_scope_id("UI/ImGui");
    MEMORY_TRACKING_SCOPE(scopeId);

    {
        ImGui::SetAllocatorFunctions([](size_t size, void*)
        {
            MEMORY_TRACKING_SCOPE(scopeId);

            return memory::allocate(size);
        }, [](void* ptr, void*)
        {
            return memory::deallocate(ptr);
        }, nullptr);
    }

    if (ImGui::GetCurrentContext())
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    io.Fonts->AddFontDefault();

    m_imGuiPlatform = nstl::make_unique<ImGuiPlatform>(*m_window);
    m_imGuiDrawer = nstl::make_unique<ImGuiDrawer>(*m_renderer);
}

void DemoApplication::unloadImgui()
{
    if (!ImGui::GetCurrentContext())
        return;

    m_imGuiPlatform = nullptr;
    m_imGuiDrawer = nullptr;
    ImGui::DestroyContext();
}

void DemoApplication::onKey(GlfwWindow::Action action, GlfwWindow::OldKey, char c, GlfwWindow::Modifiers mods)
{
    if (action != GlfwWindow::Action::Press && action != GlfwWindow::Action::Release)
        return;

    size_t index = static_cast<size_t>(c);
    m_keyState[index] = action == GlfwWindow::Action::Press;
    m_modifiers = mods;

    if (c == '`' && action == GlfwWindow::Action::Press)
        m_debugConsole->toggle();
}

void DemoApplication::onMouseMove(tglm::vec2 const& delta)
{
    tglm::vec3 angleDelta = m_mouseSensitivity * tglm::vec3{ -delta.y, -delta.x, 0.0f };
    tglm::quat rotationDelta = createRotation(angleDelta);

    m_cameraTransform.rotation *= rotationDelta;
}

void DemoApplication::createDemoScene(cgltf_data const& gltfModel, cgltf_scene const& gltfScene) const
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Hierarchy");
    MEMORY_TRACKING_SCOPE(scopeId);

    for (size_t i = 0; i < gltfScene.nodes_count; i++)
    {
        size_t nodeIndex = findIndex(gltfScene.nodes[i], gltfModel.nodes, gltfModel.nodes_count);
        createDemoObjectRecursive(gltfModel, nodeIndex, tglm::mat4::identity());
    }
}

void DemoApplication::createDemoObjectRecursive(cgltf_data const& gltfModel, size_t nodeIndex, tglm::mat4 parentTransform) const
{
    cgltf_node const& gltfNode = gltfModel.nodes[nodeIndex];

    tglm::mat4 nodeTransform = parentTransform * createMatrix(gltfNode);

    if (gltfNode.mesh)
    {
        size_t meshIndex = findIndex(gltfNode.mesh, gltfModel.meshes, gltfModel.meshes_count);
        m_sceneDrawer->addMeshInstance(m_gltfResources->demoMeshes[meshIndex], nodeTransform, { 1, 1, 1, 1 });
    }

    for (size_t i = 0; i < gltfNode.children_count; i++)
    {
        cgltf_node const* gltfChildNode = gltfNode.children[i];
        createDemoObjectRecursive(gltfModel, findIndex(gltfChildNode, gltfModel.nodes, gltfModel.nodes_count), nodeTransform);
    }
}

void DemoApplication::clearScene()
{
    if (m_gltfResources)
    {
        // TODO implement

        m_gltfResources = {};
    }

    if (m_editorGltfResources)
    {
        // TODO implement

        m_editorGltfResources = {};
    }
}

bool DemoApplication::loadScene(nstl::string_view gltfPath)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load");
    MEMORY_TRACKING_SCOPE(scopeId);

    clearScene();

    if (gltfPath.empty())
        return false;

    m_currentScenePath = gltfPath;

    nstl::string buffer;

    {
        fs::file f{ gltfPath, fs::open_mode::read };
        buffer.resize(f.size());
        f.read(buffer.data(), buffer.size());
    }

    cgltf_options options = {};
    cgltf_data* data = nullptr;

    cgltf_result result = cgltf_parse(&options, buffer.data(), buffer.size(), &data);
    nstl::scope_exit freeCgltf = [&data]() { cgltf_free(data); data = nullptr; };

    if (result != cgltf_result_success)
        return false;

    nstl::string basePath = "";
    if (auto pos = gltfPath.find_last_of("/\\"); pos != nstl::string::npos)
        basePath = gltfPath.substr(0, pos + 1);

    cgltf_load_buffers(&options, data, basePath.c_str());

    assert(data);
    loadGltfModel(basePath, *data);

    return true;
}

bool DemoApplication::loadGltfModel(nstl::string_view basePath, cgltf_data const& model)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/GLTF");
    static auto shadersScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Shaders");
    static auto buffersScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Buffers");
    static auto samplersScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Samplers");
    static auto imagesScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Images");
    static auto texturesScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Textures");
    static auto materialsScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Materials");
    static auto meshesScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Meshes");
    static auto camerasScopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Cameras");

    MEMORY_TRACKING_SCOPE(scopeId);

    m_gltfResources = nstl::make_unique<GltfResources>();

    for (size_t i = 0; i < model.extensions_required_count; i++)
        logging::warn("GLTF requires extension '{}'", model.extensions_required[i]);

    for (size_t i = 0; i < model.textures_count; i++)
    {
        MEMORY_TRACKING_SCOPE(texturesScopeId);

        cgltf_texture const& gltfTexture = model.textures[i];

        cgltf_image const& gltfImage = *gltfTexture.image;
        assert(gltfImage.uri && !gltfImage.buffer_view);

        nstl::string imagePath = basePath + gltfImage.uri;

        m_gltfResources->demoTextures.push_back(m_sceneDrawer->createTexture(vkc::utils::readBinaryFile(imagePath)));
    }

    for (size_t i = 0; i < model.materials_count; i++)
    {
        MEMORY_TRACKING_SCOPE(materialsScopeId);

        cgltf_material const& gltfMaterial = model.materials[i];

        cgltf_pbr_metallic_roughness const& gltfRoughness = gltfMaterial.pbr_metallic_roughness;

        tglm::vec4 color = createColor(gltfRoughness.base_color_factor);

        auto getTexture = [this, &model](cgltf_texture* texture) -> DemoTexture*
        {
            if (!texture)
                return nullptr;

            size_t index = findIndex(texture, model.textures, model.textures_count);
            return m_gltfResources->demoTextures[index];
        };

        m_gltfResources->demoMaterials.push_back(m_sceneDrawer->createMaterial(color, getTexture(gltfRoughness.base_color_texture.texture), getTexture(gltfMaterial.normal_texture.texture), gltfMaterial.double_sided));
    }

    for (size_t meshIndex = 0; meshIndex < model.meshes_count; meshIndex++)
    {
        MEMORY_TRACKING_SCOPE(meshesScopeId);

        cgltf_mesh const& gltfMesh = model.meshes[meshIndex];

        cgltf_buffer* buffer = nullptr;

        nstl::vector<DemoSceneDrawer::PrimitiveParams> primitiveParams;

        for (size_t primitiveIndex = 0; primitiveIndex < gltfMesh.primitives_count; primitiveIndex++)
        {
            DemoSceneDrawer::PrimitiveParams& params = primitiveParams.emplace_back();

            cgltf_primitive const& gltfPrimitive = gltfMesh.primitives[primitiveIndex];

            size_t materialIndex = findIndex(gltfPrimitive.material, model.materials, model.materials_count);

            cgltf_accessor const* gltfIndexAccessor = gltfPrimitive.indices;
            assert(gltfIndexAccessor);
            cgltf_buffer_view const* gltfIndexBufferView = gltfIndexAccessor->buffer_view;
            assert(gltfIndexBufferView);

            if (!buffer)
                buffer = gltfIndexBufferView->buffer;

            assert(gltfIndexBufferView->buffer == buffer);

            auto findIndexType = [](int gltfComponentType)
            {
                switch (gltfComponentType)
                {
                case cgltf_component_type_r_8u:
                    assert(false);
                    break;
                case cgltf_component_type_r_16u:
                    return gfx::index_type::uint16;
                case cgltf_component_type_r_32u:
                    return gfx::index_type::uint32;
                }

                assert(false);
                return gfx::index_type::uint16;
            };

            assert(gltfPrimitive.type == cgltf_primitive_type_triangles);
            params.topology = gfx::vertex_topology::triangles;
            params.material = m_gltfResources->demoMaterials[materialIndex];
            params.indexBufferOffset = gltfIndexBufferView->offset + gltfIndexAccessor->offset;
            params.indexType = findIndexType(gltfIndexAccessor->component_type);
            params.indexCount = gltfIndexAccessor->count;

            for (size_t attributeIndex = 0; attributeIndex < gltfPrimitive.attributes_count; attributeIndex++)
            {
                cgltf_attribute const& gltfAttribute = gltfPrimitive.attributes[attributeIndex];

                nstl::string_view name = gltfAttribute.name;

                nstl::optional<size_t> location = findAttributeLocation(name);

                if (!location)
                {
                    logging::warn("Skipping attribute '{}'", name);
                    continue;
                }

                cgltf_accessor const* gltfVertexAccessor = gltfAttribute.data;
                assert(gltfVertexAccessor);
                cgltf_buffer_view const* gltfVertexBufferView = gltfVertexAccessor->buffer_view;
                assert(gltfVertexBufferView);

                assert(gltfVertexBufferView->buffer == buffer); // Multiple mesh buffers aren't supported

                if (name == "COLOR_0")
                    params.hasColor = true;
                if (name == "TEXCOORD_0")
                    params.hasUv = true;
                if (name == "NORMAL")
                    params.hasNormal = true;
                if (name == "TANGENT")
                    params.hasTangent = true;

                gfx::attribute_type attributeType = findAttributeType(gltfVertexAccessor->type, gltfVertexAccessor->component_type);

                size_t stride = gltfVertexBufferView->stride;
                if (stride == 0)
                    stride = getAttributeByteSize(attributeType);

                params.attributes.push_back({
                    .location = *location,
                    .bufferOffset = gltfVertexBufferView->offset + gltfVertexAccessor->offset,
                    .stride = stride,
                    .type = attributeType,
                });
            }
        }

        assert(buffer->data);
        assert(buffer->size > 0);

        m_gltfResources->demoMeshes.push_back(m_sceneDrawer->createMesh({ buffer->data, buffer->size }, primitiveParams));
    }

    assert(model.scene);
    createDemoScene(model, *model.scene);

    return true;
}

bool DemoApplication::editorLoadScene(editor::assets::Uuid sceneId)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor");
    static auto shadersScopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Shader");

    MEMORY_TRACKING_SCOPE(scopeId);

    m_editorGltfResources = nstl::make_unique<EditorGltfResources>();

    editor::assets::SceneData scene = m_assetDatabase->loadScene(sceneId);

    for (editor::assets::ObjectDescription const& object : scene.objects)
    {
        if (!object.mesh)
            continue;

        auto calculateTransform = [&object, &scene]()
        {
            tglm::mat4 transform = tglm::mat4::identity();

            editor::assets::ObjectDescription const* obj = &object;

            while (obj)
            {
                transform = createMatrix(*obj) * transform;

                obj = obj->parentIndex ? &scene.objects[*obj->parentIndex] : nullptr;
            }

            return transform;
        };

        tglm::mat4 matrix = calculateTransform();

        editor::assets::Uuid id = object.mesh->id;
        if (m_editorGltfResources->demoMeshes.find(id) == m_editorGltfResources->demoMeshes.end())
            editorLoadMesh(id);

        m_sceneDrawer->addMeshInstance(m_editorGltfResources->demoMeshes[id], calculateTransform(), { 1, 1, 1, 1 });
    }

    return true;
}

void DemoApplication::editorLoadImage(editor::assets::Uuid id)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Image");
    MEMORY_TRACKING_SCOPE(scopeId);

    m_editorGltfResources->demoTextures[id] = m_sceneDrawer->createTexture(m_assetDatabase->loadImage(id));
}

void DemoApplication::editorLoadMaterial(editor::assets::Uuid id)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Material");

    MEMORY_TRACKING_SCOPE(scopeId);

    editor::assets::MaterialData materialData = m_assetDatabase->loadMaterial(id);

    tglm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

    if (materialData.baseColor)
        color = tglm::vec4{ materialData.baseColor->data };

    auto getTexture = [this](nstl::optional<editor::assets::TextureData> const& data) -> DemoTexture*
    {
        if (!data)
            return nullptr;

        if (m_editorGltfResources->demoTextures.find(data->image) == m_editorGltfResources->demoTextures.end())
            editorLoadImage(data->image);

        return m_editorGltfResources->demoTextures[data->image];
    };

    m_editorGltfResources->demoMaterials[id] = m_sceneDrawer->createMaterial(color, getTexture(materialData.baseColorTexture), getTexture(materialData.normalTexture), materialData.doubleSided);
}

void DemoApplication::editorLoadMesh(editor::assets::Uuid id)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Mesh");

    MEMORY_TRACKING_SCOPE(scopeId);

    editor::assets::MeshData meshData = m_assetDatabase->loadMesh(id);

    nstl::vector<DemoSceneDrawer::PrimitiveParams> primitiveParams;
    for (editor::assets::PrimitiveDescription const& primitiveData : meshData.primitives)
    {
        editor::assets::Uuid materialId = primitiveData.material;
        if (m_editorGltfResources->demoMaterials.find(materialId) == m_editorGltfResources->demoMaterials.end())
            editorLoadMaterial(materialId);

        DemoSceneDrawer::PrimitiveParams& params = primitiveParams.emplace_back();

        auto findIndexType = [](editor::assets::DataComponentType componentType)
        {
            switch (componentType)
            {
            case editor::assets::DataComponentType::Int8:
                assert(false);
                break;
            case editor::assets::DataComponentType::UInt16:
                return gfx::index_type::uint16;
            case editor::assets::DataComponentType::UInt32:
                return gfx::index_type::uint32;
            default:
                assert(false);
            }

            assert(false);
            return gfx::index_type::uint16;
        };

        assert(primitiveData.topology == editor::assets::Topology::Triangles); // TODO implement
        assert(primitiveData.indices.type == editor::assets::DataType::Scalar);
        params.topology = gfx::vertex_topology::triangles;
        params.material = m_editorGltfResources->demoMaterials[materialId];
        params.indexBufferOffset = primitiveData.indices.bufferOffset;
        params.indexType = findIndexType(primitiveData.indices.componentType);
        params.indexCount = primitiveData.indices.count;

        for (editor::assets::VertexAttributeDescription const& attributeData : primitiveData.vertexAttributes)
        {
            if (attributeData.index != 0)
            {
                logging::warn("Skipping attribute {}: non-zero index {} not supported", attributeData.semantic, attributeData.index);
                continue;
            }

            nstl::optional<size_t> location = findAttributeLocation(attributeData.semantic);

            if (!location)
            {
                logging::warn("Skipping attribute {}: unknown location", attributeData.semantic);
                continue;
            }

            if (attributeData.semantic == editor::assets::AttributeSemantic::Color)
                params.hasColor = true;
            if (attributeData.semantic == editor::assets::AttributeSemantic::Texcoord)
                params.hasUv = true;
            if (attributeData.semantic == editor::assets::AttributeSemantic::Normal)
                params.hasNormal = true;
            if (attributeData.semantic == editor::assets::AttributeSemantic::Tangent)
                params.hasTangent = true;


            params.attributes.push_back({
                .location = *location,
                .bufferOffset = attributeData.accessor.bufferOffset,
                .stride = attributeData.accessor.stride,
                .type = newFindAttributeType(attributeData.accessor.type, attributeData.accessor.componentType),
            });
        }
    }

    nstl::blob data = m_assetDatabase->loadMeshData(id);

    m_editorGltfResources->demoMeshes[id] = m_sceneDrawer->createMesh(data, primitiveParams);
}

void DemoApplication::updateUI(float frameTime)
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

    m_imGuiPlatform->update(frameTime);

    ImGui::NewFrame();

    if (m_drawImguiDemo)
        ImGui::ShowDemoWindow(&m_drawImguiDemo);
    if (m_drawImguiDebugger)
        ImGui::ShowMetricsWindow(&m_drawImguiDebugger);
    if (m_drawImguiStyleEditor)
        ImGui::ShowStyleEditor();

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
        ImGui::Text("Uptime %.3f", m_time);
        if (ImGui::Button(m_paused ? "Unpause" : "Pause"))
        {
            m_paused = !m_paused;
        }
        static nstl::vector<nstl::string> texts = {
            "Lorem ipsum dolor sit amet",
            "consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore",
            "et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation",
        };
        static size_t nextIndex = 0;
        if (ImGui::Button("Add notification"))
        {
            m_notifications->add(texts[nextIndex % texts.size()]);
            nextIndex++;
        }
        ImGui::End();
    }

    m_notifications->draw();
    m_debugConsole->draw();
    m_memoryViewer->draw();

    // TODO fix this logic
    m_window->setCanCaptureCursor(!io.WantCaptureMouse);
}

void DemoApplication::drawFrame()
{
    m_services.debugDraw().beginFrame();

    update();

    draw();

    m_services.debugDraw().endFrame();

    m_fpsDrawnFrames++;
}

void DemoApplication::update()
{
    m_time = m_appTime.getTime();

    if (m_frameTimer.getTime() > m_fpsUpdatePeriod)
    {
        float multiplier = 1.0f / static_cast<float>(m_fpsDrawnFrames);
        m_lastFrameTime = m_frameTimer.loop() * multiplier;
        m_fpsDrawnFrames = 0;
    }

    float const dt = m_lastFrameTime;

    m_notifications->update(dt);

    updateUI(m_lastFrameTime);
    updateScene(dt);
    updateCamera(dt);
}

void DemoApplication::updateScene(float)
{
    m_lightParameters.rotation = tglm::quat::from_euler_xyz(tglm::radians({ 45.0f * m_time, 0, 0 }));
    m_lightParameters.position.x = 2.0f * sinf(2.0f * m_time);

    m_services.debugDraw().box(m_lightParameters.position, tglm::quat::identity(), tglm::vec3{ 0.1f }, { 1.0f, 0.0f, 0.0f }, -1.0f);
}

void DemoApplication::updateCamera(float dt)
{
    // TODO handle input properly
    if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
        return;

    tglm::vec3 right = m_cameraTransform.rotation.rotate(tglm::vec3(1.0f, 0.0f, 0.0f));
    tglm::vec3 forward = m_cameraTransform.rotation.rotate(tglm::vec3(0.0f, 0.0f, -1.0f));
    tglm::vec3 up = m_cameraTransform.rotation.rotate(tglm::vec3(0.0f, 1.0f, 0.0f));

    tglm::vec3 posDelta = { 0.0f, 0.0f, 0.0f };

    if (m_keyState['A'])
        posDelta += -right;
    if (m_keyState['D'])
        posDelta += right;
    if (m_keyState['S'])
        posDelta += -forward;
    if (m_keyState['W'])
        posDelta += forward;
    if (m_keyState['Q'])
        posDelta += -up;
    if (m_keyState['E'])
        posDelta += up;

    m_cameraTransform.position += m_cameraSpeed * dt * posDelta.normalized();
}

void DemoApplication::draw()
{
    ImGui::Render();

    if (m_window->isIconified())
        return;

    m_renderer->begin_resource_update();

    {
        auto aspectRatio = m_renderer->get_main_framebuffer_aspect();

        auto lightAspectRatio = 1.0f * SHADOWMAP_RESOLUTION / SHADOWMAP_RESOLUTION;
        auto lightNearZ = 0.1f;
        auto lightFarZ = 10000.0f;

        ShaderViewProjectionData shadowmapViewProjectionData = {
            .view = (tglm::translated(tglm::mat4::identity(), m_lightParameters.position) * m_lightParameters.rotation.to_mat4()).inversed(), // TODO rewrite this operation
            .projection = tglm::perspective(tglm::radians(SHADOWMAP_FOV), lightAspectRatio, lightNearZ, lightFarZ),
        };
        shadowmapViewProjectionData.projection.data[1][1] *= -1; // TODO fix this hack

        m_renderer->buffer_upload_sync(m_shadowmapViewProjectionData, { &shadowmapViewProjectionData, sizeof(shadowmapViewProjectionData) });

        ShaderViewProjectionData viewProjectionData = {
            .view = (tglm::translated(tglm::mat4::identity(), m_cameraTransform.position) * m_cameraTransform.rotation.to_mat4()).inversed(), // TODO rewrite this operation
            .projection = tglm::perspective(tglm::radians(m_cameraParameters.fov), aspectRatio, m_cameraParameters.nearZ, m_cameraParameters.farZ),
        };
        viewProjectionData.projection.data[1][1] *= -1; // TODO fix this hack

        m_renderer->buffer_upload_sync(m_viewProjectionData, { &viewProjectionData, sizeof(viewProjectionData) });

        ShaderLightData lightData = {
            .lightViewProjection = shadowmapViewProjectionData.projection * shadowmapViewProjectionData.view,
            .lightPosition = viewProjectionData.view * tglm::vec4(m_lightParameters.position, 1.0f),
            .lightColor = m_lightParameters.intensity * m_lightParameters.color,
        };

        m_renderer->buffer_upload_sync(m_lightData, { &lightData, sizeof(lightData) });
    }

    m_services.debugDraw().updateResources(*m_renderer);

    if (m_imGuiDrawer)
        m_imGuiDrawer->updateResources(*m_renderer);

    m_sceneDrawer->updateResources();

    m_renderer->begin_frame();

    m_renderer->renderpass_begin({
        .renderpass = m_shadowRenderpass,
        .framebuffer = m_shadowFramebuffer,
    });

    m_sceneDrawer->draw(true, m_cameraDescriptorGroup, m_shadowmapCameraDescriptorGroup);

    m_renderer->renderpass_end();

    m_renderer->renderpass_begin({
        .renderpass = m_renderer->get_main_renderpass(),
        .framebuffer = m_renderer->acquire_main_framebuffer(),
    });

    m_sceneDrawer->draw(false, m_cameraDescriptorGroup, m_shadowmapCameraDescriptorGroup);

    m_services.debugDraw().draw(*m_renderer, m_cameraDescriptorGroup);

    // TODO should be in its own renderpass
    if (m_imGuiDrawer)
        m_imGuiDrawer->draw(*m_renderer);

    m_renderer->renderpass_end();

    m_renderer->submit();
}
