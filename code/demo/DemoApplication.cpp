#include "DemoApplication.h"

#include "stb_image.h"
#include "dds-ktx.h"
#include "cgltf.h"

#include "tiny_ktx/tiny_ktx.h"

#include "tglm/tglm.h" // TODO only include what is needed

#include "imgui.h"

#include <vulkan/vulkan.h>

#include "vko/SamplerProperties.h"
#include "vko/DebugMessage.h"
#include "vko/ShaderModuleProperties.h"

#include "ImGuiPlatform.h"
#include "ImGuiDrawer.h"

#include "ShaderPackage.h"

#include "vkgfx/Renderer.h"
#include "vkgfx/ResourceManager.h"
#include "vkgfx/Handles.h"
#include "vkgfx/ImageMetadata.h"
#include "vkgfx/Texture.h"
#include "vkgfx/Mesh.h"
#include "vkgfx/Material.h"
#include "vkgfx/BufferMetadata.h"
#include "vkgfx/TestObject.h"

#include "gfx/renderer.h"

#include "gfx_vk/backend.h"

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
#include "nstl/sort.h"

namespace
{
    const uint32_t TARGET_WINDOW_WIDTH = 1900;
    const uint32_t TARGET_WINDOW_HEIGHT = 1000;

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

    struct ImageData
    {
        struct MipData
        {
            size_t offset = 0;
            size_t size = 0;
        };

        size_t width = 0;
        size_t height = 0;

        vkgfx::ImageFormat format = vkgfx::ImageFormat::R8G8B8A8;

        nstl::vector<unsigned char> bytes;
        nstl::vector<MipData> mips;
    };

    nstl::optional<ImageData> loadWithStbImage(nstl::blob_view bytes)
    {
        static auto scopeId = memory::tracking::create_scope_id("Image/Load/STBI");
        MEMORY_TRACKING_SCOPE(scopeId);

        assert(bytes.size() <= INT_MAX);

        int w = 0, h = 0, comp = 0, req_comp = 4;
        unsigned char* data = stbi_load_from_memory(bytes.ucdata(), static_cast<int>(bytes.size()), &w, &h, &comp, req_comp);
        int bits = 8;

        if (!data)
            return {};

        if (w < 1 || h < 1)
        {
            stbi_image_free(data);
            return {};
        }

        if (req_comp != 0)
        {
            // loaded data has `req_comp` channels(components)
            comp = req_comp;
        }

        ImageData imageData;

        imageData.width = w;
        imageData.height = h;

        // TODO refactor this mess and support other pixel types
        imageData.format = [](int bits, int comp) {
            if (bits == 8 && comp == 4)
                return vkgfx::ImageFormat::R8G8B8A8;

            assert(false);
            return vkgfx::ImageFormat::R8G8B8A8;
        }(bits, comp);

        size_t dataSize = static_cast<size_t>(w * h * comp) * size_t(bits / 8);
        imageData.bytes.resize(dataSize);
        nstl::copy(data, data + dataSize, imageData.bytes.begin());
        stbi_image_free(data);

        imageData.mips = { { 0, imageData.bytes.size() } };

        return imageData;
    }

    nstl::optional<ImageData> loadWithDdspp(nstl::blob_view bytes)
    {
        static auto scopeId = memory::tracking::create_scope_id("Image/Load/DDS");
        MEMORY_TRACKING_SCOPE(scopeId);

        assert(bytes.size() <= INT_MAX);

        ddsktx_texture_info info{};
        if (!ddsktx_parse(&info, bytes.data(), static_cast<int>(bytes.size()), nullptr))
            return {};

        assert(info.bpp > 0);
        assert(info.bpp % 4 == 0);

        ImageData imageData;

        imageData.width = info.width;
        imageData.height = info.height;

        imageData.format = [](ddsktx_format format)
        {
            switch (format)
            {
            case DDSKTX_FORMAT_BC1:
                return vkgfx::ImageFormat::BC1_UNORM;
            case DDSKTX_FORMAT_BC3:
                return vkgfx::ImageFormat::BC3_UNORM;
            case DDSKTX_FORMAT_BC5:
                return vkgfx::ImageFormat::BC5_UNORM;
            default: // TODO implement other formats
                assert(false);
            }

            assert(false);
            return vkgfx::ImageFormat::BC1_UNORM;
        }(info.format);

        // TODO don't include the header
        imageData.bytes.resize(bytes.size());
        memcpy(imageData.bytes.data(), bytes.data(), bytes.size());

        for (int mip = 0; mip < info.num_mips; mip++)
        {
            ddsktx_sub_data mipInfo;
            ddsktx_get_sub(&info, &mipInfo, bytes.data(), static_cast<int>(bytes.size()), 0, 0, mip);

            assert(mipInfo.buff > bytes.data());
            size_t offset = static_cast<unsigned char const*>(mipInfo.buff) - bytes.ucdata();

            imageData.mips.push_back({ offset, static_cast<size_t>(mipInfo.size_bytes) });
        }

        return imageData;
    }

    nstl::optional<ImageData> loadWithKtx(nstl::blob_view bytes)
    {
        static auto scopeId = memory::tracking::create_scope_id("Image/Load/KTX");
        MEMORY_TRACKING_SCOPE(scopeId);

        class memory_stream : public tiny_ktx::input_stream
        {
        public:
            memory_stream(nstl::blob_view bytes) : m_bytes(bytes) {}

            bool read(void* dest, size_t size) override
            {
                if (!dest)
                    return false;

                if (m_position + size > m_bytes.size())
                    return false;

                memcpy(dest, m_bytes.ucdata() + m_position, size);
                m_position += size;
                return true;
            }

        private:
            nstl::blob_view m_bytes;
            size_t m_position = 0;
        };

        memory_stream stream{ bytes };

        tiny_ktx::image_header header;
        if (!tiny_ktx::parse_header(&header, stream))
            return {};

        assert(header.layer_count == 0 || header.layer_count == 1);
        assert(header.face_count == 1);
        assert(header.pixel_depth == 0 || header.pixel_depth == 1);

        ImageData imageData;

        imageData.width = header.pixel_width;
        imageData.height = header.pixel_height;

        imageData.format = [](VkFormat format)
        {
            switch (format)
            {
            case VK_FORMAT_R8G8B8_UNORM:
                return vkgfx::ImageFormat::R8G8B8;
            case VK_FORMAT_R8G8B8A8_UNORM:
                return vkgfx::ImageFormat::R8G8B8A8;
            default: // TODO implement other formats
                assert(false);
            }

            assert(false);
            return vkgfx::ImageFormat::R8G8B8A8;
        }(static_cast<VkFormat>(header.vk_format));

        size_t mipsCount = tiny_ktx::get_level_count(header);

        nstl::vector<tiny_ktx::image_level_info> levelIndex{ mipsCount }; // TODO: static or hybrid vector
        if (!tiny_ktx::load_image_level_index(levelIndex.data(), levelIndex.size(), header, stream))
            return {};

        size_t dataOffset = levelIndex[mipsCount - 1].byte_offset;
        size_t dataSize = levelIndex[0].byte_offset + levelIndex[0].byte_length - dataOffset;
        imageData.bytes.resize(dataSize);

        assert(dataOffset + dataSize <= bytes.size());
        memcpy(imageData.bytes.data(), bytes.ucdata() + dataOffset, dataSize);

        for (size_t i = 0; i < mipsCount; i++)
        {
            size_t mipOffset = levelIndex[i].byte_offset - dataOffset;
            size_t mipSize = levelIndex[i].byte_length;

            imageData.mips.push_back({ mipOffset, mipSize });
        }

        return imageData;
    }

    nstl::optional<ImageData> loadImage(nstl::blob_view bytes)
    {
        static auto scopeId = memory::tracking::create_scope_id("Image/Load");
        MEMORY_TRACKING_SCOPE(scopeId);

        if (auto data = loadWithDdspp(bytes))
            return data;

        if (auto data = loadWithStbImage(bytes))
            return data;

        if (auto data = loadWithKtx(bytes))
            return data;

        return {};
    }

    vkgfx::AttributeType findAttributeType(cgltf_type gltfAttributeType, cgltf_component_type gltfComponentType)
    {
        if (gltfAttributeType == cgltf_type_vec2 && gltfComponentType == cgltf_component_type_r_32f)
            return vkgfx::AttributeType::Vec2f;
        if (gltfAttributeType == cgltf_type_vec3 && gltfComponentType == cgltf_component_type_r_32f)
            return vkgfx::AttributeType::Vec3f;
        if (gltfAttributeType == cgltf_type_vec4 && gltfComponentType == cgltf_component_type_r_32f)
            return vkgfx::AttributeType::Vec4f;

        assert(false);
        return vkgfx::AttributeType::Vec4f;
    }

    vkgfx::AttributeType findAttributeType(editor::assets::DataType dataType, editor::assets::DataComponentType componentType)
    {
        if (dataType == editor::assets::DataType::Vec2 && componentType == editor::assets::DataComponentType::Float)
            return vkgfx::AttributeType::Vec2f;
        if (dataType == editor::assets::DataType::Vec3 && componentType == editor::assets::DataComponentType::Float)
            return vkgfx::AttributeType::Vec3f;
        if (dataType == editor::assets::DataType::Vec4 && componentType == editor::assets::DataComponentType::Float)
            return vkgfx::AttributeType::Vec4f;

        assert(false);
        return vkgfx::AttributeType::Vec4f;
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

    size_t getAttributeByteSize(vkgfx::AttributeType type)
    {
        size_t gltfFloatSize = 4;

        switch (type)
        {
        case vkgfx::AttributeType::Vec2f:
            return 2 * gltfFloatSize;
        case vkgfx::AttributeType::Vec3f:
            return 3 * gltfFloatSize;
        case vkgfx::AttributeType::Vec4f:
            return 4 * gltfFloatSize;
        case vkgfx::AttributeType::UInt32:
            return 4;
        case vkgfx::AttributeType::Mat2f:
            return 4 * gltfFloatSize;
        case vkgfx::AttributeType::Mat3f:
            return 9 * gltfFloatSize;
        case vkgfx::AttributeType::Mat4f:
            return 16 * gltfFloatSize;
        }

        assert(false);
        return 0;
    }

    nstl::optional<size_t> findAttributeLocation(nstl::string_view name)
    {
        static nstl::vector<nstl::string_view> const attributeNames = { "POSITION", "COLOR_0", "TEXCOORD_0", "NORMAL", "TANGENT" }; // TODO move to the shader metadata

        auto it = attributeNames.find(name);

        if (it != attributeNames.end())
            return it - attributeNames.begin();

        return {};
    }

    nstl::optional<size_t> findAttributeLocation(editor::assets::AttributeSemantic semantic)
    {
        using editor::assets::AttributeSemantic;

        static nstl::vector<AttributeSemantic> const attributeSemantics = { AttributeSemantic::Position, AttributeSemantic::Color, AttributeSemantic::Texcoord, AttributeSemantic::Normal, AttributeSemantic::Tangent }; // TODO move to the shader metadata

        auto it = attributeSemantics.find(semantic);

        if (it != attributeSemantics.end())
            return it - attributeSemantics.begin();

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
        bool success = m_services.debugConsole().execute(line);
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
        bool success = m_services.debugConsole().execute(line);
        assert(success);
    }

    m_keyState.resize(1 << 8 * sizeof(char), false);

    m_window = nstl::make_unique<GlfwWindow>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
    m_window->addOldKeyCallback([this](GlfwWindow::Action action, GlfwWindow::OldKey key, char c, GlfwWindow::Modifiers modifiers) { onKey(action, key, c, modifiers); });
    m_window->addOldMouseDeltaCallback([this](float deltaX, float deltaY) { onMouseMove({ deltaX, deltaY }); });

    gfx_vk::config config = {
        .descriptors = {},
        .name = "Vulkan Demo",
        .enable_validation = m_validationEnabled,
    };
    auto backend = nstl::make_unique<gfx_vk::backend>(*m_window, config);
    m_newRenderer = nstl::make_unique<gfx::renderer>(nstl::move(backend));

    m_viewProjectionData = m_newRenderer->create_buffer({
        .size = sizeof(ShaderViewProjectionData),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = false, // TODO change
    });

    m_lightData = m_newRenderer->create_buffer({
        .size = sizeof(ShaderLightData),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = false, // TODO change
    });

    {
        auto aspectRatio = m_newRenderer->get_main_framebuffer_aspect();

        ShaderViewProjectionData viewProjectionData = {
            .view = (tglm::translated(tglm::mat4::identity(), m_cameraTransform.position) * m_cameraTransform.rotation.to_mat4()).inversed(), // TODO rewrite this operation
            .projection = tglm::perspective(tglm::radians(m_cameraParameters.fov), aspectRatio, m_cameraParameters.nearZ, m_cameraParameters.farZ),
        };

        viewProjectionData.projection.data[1][1] *= -1; // TODO fix this hack

        m_newRenderer->buffer_upload_sync(m_viewProjectionData, { &viewProjectionData, sizeof(viewProjectionData) });

        auto rotation = tglm::quat::from_euler_xyz(tglm::radians({ 0, 0, 0 })); // TODO fix

        auto lightAspectRatio = 1.0f * SHADOWMAP_RESOLUTION / SHADOWMAP_RESOLUTION;
        auto nearZ = 0.1f;
        auto farZ = 10000.0f;

        tglm::mat4 lightView = (tglm::translated(tglm::mat4::identity(), m_lightParameters.position) * rotation.to_mat4()).inversed(); // TODO rewrite this operation
        tglm::mat4 lightProjection = tglm::perspective(tglm::radians(SHADOWMAP_FOV), lightAspectRatio, nearZ, farZ);

        ShaderLightData lightData = {
            .lightViewProjection = lightProjection * lightView,
            .lightPosition = viewProjectionData.view * tglm::vec4(m_lightParameters.position, 1.0f),
            .lightColor = m_lightParameters.intensity * m_lightParameters.color,
        };

        m_newRenderer->buffer_upload_sync(m_lightData, { &lightData, sizeof(lightData) });
    }

    m_cameraDescriptorGroup = m_newRenderer->create_descriptorgroup({
        .entries = nstl::array{
            gfx::descriptorgroup_entry{0, {m_viewProjectionData}},
            gfx::descriptorgroup_entry{1, {m_lightData}},
        }
    });

    m_shadowRenderpass = m_newRenderer->create_renderpass({
        .color_attachment_formats = {},
        .depth_stencil_attachment_format = gfx::image_format::d32_float,

        .has_presentable_images = false,
        .keep_depth_values_after_renderpass = true,
    });

    m_shadowImage = m_newRenderer->create_image({
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION,
        .format = gfx::image_format::d32_float,
        .type = gfx::image_type::depth,
        .usage = gfx::image_usage::depth_sampled,
    });

    m_shadowFramebuffer = m_newRenderer->create_framebuffer({
        .attachments = nstl::array{ m_shadowImage },
        .renderpass = m_shadowRenderpass,
    });

    auto messageCallback = [](vko::DebugMessage m)
    {
        // TODO don't log "Info" level to the console
// 		if (m.level == vko::DebugMessage::Level::Info)
// 			logging::info("{}", m.text);
        if (m.level == vko::DebugMessage::Level::Warning)
            logging::warn("{}", m.text);
        if (m.level == vko::DebugMessage::Level::Error)
            logging::error("{}", m.text);

        assert(m.level != vko::DebugMessage::Level::Error);
    };

    m_renderer = nstl::make_unique<vkgfx::Renderer>("Vulkan demo with new API", m_validationEnabled, *m_window, messageCallback);

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
    createTestResources();
}

void DemoApplication::createResources()
{
    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    m_defaultVertexShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/shader.vert");
    m_newDefaultVertexShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/new/shader.vert");
    m_defaultFragmentShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/shader.frag");
    m_newDefaultFragmentShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/new/shader.frag");

    m_shadowmapVertexShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/shadowmap.vert");
    m_newShadowmapVertexShader = nstl::make_unique<ShaderPackage>("data/shaders/packaged/new/shadowmap.vert");

    m_defaultSampler = resourceManager.createSampler(vko::SamplerFilterMode::Linear, vko::SamplerFilterMode::Linear, vko::SamplerWrapMode::Repeat, vko::SamplerWrapMode::Repeat);
    m_newDefaultSampler = m_newRenderer->create_sampler({});

    m_defaultAlbedoImage = resourceManager.createImage(vkgfx::ImageMetadata{
        .width = 1,
        .height = 1,
        .byteSize = 4,
        .format = vkgfx::ImageFormat::R8G8B8A8,
    });
    resourceManager.uploadImage(m_defaultAlbedoImage, nstl::array<unsigned char, 4>{ 0xff, 0xff, 0xff, 0xff });
    m_defaultAlbedoTexture = resourceManager.createTexture(vkgfx::Texture{ m_defaultAlbedoImage, m_defaultSampler });

    m_newDefaultAlbedoImage = m_newRenderer->create_image({
        .width = 1,
        .height = 1,
        .format = gfx::image_format::r8g8b8a8,
        .type = gfx::image_type::color,
        .usage = gfx::image_usage::upload_sampled,
    });
    m_newRenderer->image_upload_sync(m_newDefaultAlbedoImage, nstl::array<unsigned char, 4>{ 0xff, 0xff, 0xff, 0xff });

    m_defaultNormalMapImage = resourceManager.createImage(vkgfx::ImageMetadata{
        .width = 1,
        .height = 1,
        .byteSize = 4,
        .format = vkgfx::ImageFormat::R8G8B8A8,
    });
    resourceManager.uploadImage(m_defaultNormalMapImage, nstl::array<unsigned char, 4>{ 0x80, 0x80, 0xff, 0xff });
    m_defaultNormalMapTexture = resourceManager.createTexture(vkgfx::Texture{ m_defaultNormalMapImage, m_defaultSampler });

    m_newDefaultNormalMapImage = m_newRenderer->create_image({
        .width = 1,
        .height = 1,
        .format = gfx::image_format::r8g8b8a8,
        .type = gfx::image_type::color,
        .usage = gfx::image_usage::upload_sampled,
    });
    m_newRenderer->image_upload_sync(m_newDefaultNormalMapImage, nstl::array<unsigned char, 4>{ 0x80, 0x80, 0xff, 0xff });
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

void DemoApplication::onKey(GlfwWindow::Action action, GlfwWindow::OldKey key, char c, GlfwWindow::Modifiers mods)
{
    if (action != GlfwWindow::Action::Press && action != GlfwWindow::Action::Release)
        return;

    char const* separator = "";

    nstl::string_builder builder;

    for (GlfwWindow::Modifiers value : tiny_ctti::enum_values<GlfwWindow::Modifiers>())
    {
        if (mods & value)
        {
            nstl::string_view name = tiny_ctti::enum_name(value);
            builder.append(separator).append(name);
            separator = " | ";
        }
    }

    logging::info("onKey: {} {} {}: '{}'", tiny_ctti::enum_name(action), tiny_ctti::enum_name(key), builder.build(), c);

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

DemoScene DemoApplication::createDemoScene(cgltf_data const& gltfModel, cgltf_scene const& gltfScene) const
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/GLTF/Hierarchy");
    MEMORY_TRACKING_SCOPE(scopeId);

    // TODO is there a better way?
    auto findIndex = [](auto const* object, auto const* firstObject, size_t count) -> size_t
    {
        size_t index = object - firstObject;
        assert(index >= 0);
        assert(index < count);
        return static_cast<size_t>(index);
    };

    DemoScene scene;

    for (size_t i = 0; i < gltfScene.nodes_count; i++)
    {
        size_t nodeIndex = findIndex(gltfScene.nodes[i], gltfModel.nodes, gltfModel.nodes_count);
        createDemoObjectRecursive(gltfModel, nodeIndex, tglm::mat4::identity(), scene);
    }

    return scene;
}

void DemoApplication::createDemoObjectRecursive(cgltf_data const& gltfModel, size_t nodeIndex, tglm::mat4 parentTransform, DemoScene& scene) const
{
    // TODO is there a better way?
    auto findIndex = [](auto const* object, auto const* firstObject, size_t count) -> size_t
    {
        size_t index = object - firstObject;
        assert(index >= 0);
        assert(index < count);
        return static_cast<size_t>(index);
    };

    struct DemoObjectPushConstants
    {
        tglm::mat4 model;
    };

    struct DemoObjectUniformBuffer
    {
        tglm::vec4 color;
    };

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    cgltf_node const& gltfNode = gltfModel.nodes[nodeIndex];

    tglm::mat4 nodeTransform = parentTransform * createMatrix(gltfNode);

    if (gltfNode.mesh)
    {
        size_t meshIndex = findIndex(gltfNode.mesh, gltfModel.meshes, gltfModel.meshes_count);

        for (DemoPrimitive const& primitive : m_gltfResources->meshes[meshIndex].primitives)
        {
            DemoMaterial const& material = m_gltfResources->materials[primitive.metadata.materialIndex];

            vkgfx::PipelineKey pipelineKey;
            pipelineKey.renderConfig = material.metadata.renderConfig;

            // TODO think how to handle multiple descriptor set layouts properly
            pipelineKey.uniformConfigs = {
                // TODO get shared frame uniform config from the Renderer
                vkgfx::UniformConfiguration{
                    .hasBuffer = true,
                    .hasAlbedoTexture = false,
                    .hasNormalMap = false,
                    .hasShadowMap = true,
                },
                material.metadata.uniformConfig,
                vkgfx::UniformConfiguration{
                    .hasBuffer = true,
                    .hasAlbedoTexture = false,
                    .hasNormalMap = false,
                },
            };

            pipelineKey.vertexConfig = primitive.metadata.vertexConfig;

            pipelineKey.pushConstantRanges = { vkgfx::PushConstantRange{.offset = 0, .size = sizeof(DemoObjectPushConstants), } };

            // TODO reimplement
            ShaderConfiguration shaderConfiguration;
            shaderConfiguration.hasTexture = material.metadata.uniformConfig.hasAlbedoTexture;
            shaderConfiguration.hasNormalMap = material.metadata.uniformConfig.hasNormalMap;
            shaderConfiguration.hasColor = primitive.metadata.attributeSemanticsConfig.hasColor;
            shaderConfiguration.hasTexCoord = primitive.metadata.attributeSemanticsConfig.hasUv;
            shaderConfiguration.hasNormal = primitive.metadata.attributeSemanticsConfig.hasNormal;
            shaderConfiguration.hasTangent = primitive.metadata.attributeSemanticsConfig.hasTangent;

            nstl::string const* vertexShaderPath = m_defaultVertexShader->get(shaderConfiguration);
            nstl::string const* fragmentShaderPath = m_defaultFragmentShader->get(shaderConfiguration);

            assert(vertexShaderPath && fragmentShaderPath);

            vkgfx::ShaderModuleHandle vertexShaderModule = m_gltfResources->shaderModules[*vertexShaderPath];
            vkgfx::ShaderModuleHandle fragmentShaderModule = m_gltfResources->shaderModules[*fragmentShaderPath];

            pipelineKey.shaderHandles = { vertexShaderModule, fragmentShaderModule };

            vkgfx::PipelineHandle pipeline = resourceManager.getOrCreatePipeline(pipelineKey);

            vkgfx::BufferMetadata uniformBufferMetadata{
                .usage = vkgfx::BufferUsage::UniformBuffer,
                .location = vkgfx::BufferLocation::HostVisible,
                .isMutable = false,
            };
            vkgfx::BufferHandle uniformBuffer = resourceManager.createBuffer(sizeof(DemoObjectUniformBuffer), nstl::move(uniformBufferMetadata));
            m_gltfResources->additionalBuffers.push_back(uniformBuffer);

            DemoObjectUniformBuffer uniformValues;
            uniformValues.color = { 1.0f, 1.0f, 1.0f, 0.0f };
            resourceManager.uploadBuffer(uniformBuffer, nstl::blob_view{ &uniformValues, sizeof(uniformValues) });

            DemoObjectPushConstants pushConstants;
            pushConstants.model = nodeTransform;

            vkgfx::TestObject& object = scene.objects.emplace_back();
            object.mesh = primitive.handle;
            object.material = material.handle;
            object.pipeline = pipeline;
            object.uniformBuffer = uniformBuffer;
            object.pushConstants.resize(sizeof(pushConstants));
            memcpy(object.pushConstants.data(), &pushConstants, object.pushConstants.size());
        }
    }

    if (gltfNode.camera)
    {
        tglm::vec4 position;
        tglm::vec3 scale;
        tglm::quat rotation;
        tglm::decompose(nodeTransform, position, rotation, scale);

        scene.cameras.push_back(DemoCamera{
            .transform = {
                .position = position,
                .rotation = rotation,
            },
            .parametersIndex = findIndex(gltfNode.camera, gltfModel.cameras, gltfModel.cameras_count),
        });
    }

    for (size_t i = 0; i < gltfNode.children_count; i++)
    {
        cgltf_node const* gltfChildNode = gltfNode.children[i];
        createDemoObjectRecursive(gltfModel, findIndex(gltfChildNode, gltfModel.nodes, gltfModel.nodes_count), nodeTransform, scene);
    }
}

void DemoApplication::clearScene()
{
    m_demoScene = {};

    if (m_gltfResources)
    {
        m_renderer->waitIdle(); // TODO remove

        GltfResources const& resources = *m_gltfResources;
        vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

        for (auto const& handle : resources.buffers)
            resourceManager.removeBuffer(handle);
        for (auto const& handle : resources.additionalBuffers)
            resourceManager.removeBuffer(handle);
        for (auto const& handle : resources.images)
            resourceManager.removeImage(handle);
        for (auto const& material : resources.materials)
            resourceManager.removeMaterial(material.handle);
        for (auto const& mesh : resources.meshes)
            for (auto const& primitive : mesh.primitives)
                resourceManager.removeMesh(primitive.handle);
        for (auto const& handle : resources.samplers)
            resourceManager.removeSampler(handle);
        for (auto const& [name, handle] : resources.shaderModules)
            resourceManager.removeShaderModule(handle);
        for (auto const& handle : resources.textures)
            resourceManager.removeTexture(handle);

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

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    m_gltfResources = nstl::make_unique<GltfResources>();

    // TODO make more generic?
    size_t totalMeshes = 0;
    for (size_t i = 0; i < model.meshes_count; i++)
        totalMeshes += model.meshes[i].primitives_count;
    resourceManager.reserveMoreMeshes(totalMeshes);
    resourceManager.reserveMoreBuffers(model.buffers_count + model.materials_count + totalMeshes);

    m_gltfResources->additionalBuffers.reserve(model.materials_count + totalMeshes);

    // TODO is there a better way?
    auto findIndex = [](auto const* object, auto const* firstObject, size_t count) -> size_t
    {
        size_t index = object - firstObject;
        assert(index >= 0);
        assert(index < count);
        return static_cast<size_t>(index);
    };

    for (size_t i = 0; i < model.extensions_required_count; i++)
        logging::warn("GLTF requires extension '{}'", model.extensions_required[i]);

    // TODO move somewhere else? Doesn't seem to be related to the GLTF model
    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultVertexShader->getAll())
    for (auto const& pair : m_defaultVertexShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Vertex, "main");
        m_gltfResources->shaderModules.insert_or_assign(modulePath, handle);
    }
    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultFragmentShader->getAll())
    for (auto const& pair : m_defaultFragmentShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Fragment, "main");
        m_gltfResources->shaderModules.insert_or_assign(modulePath, handle);
    }

    m_gltfResources->buffers.reserve(model.buffers_count);
    for (size_t i = 0; i < model.buffers_count; i++)
    {
        MEMORY_TRACKING_SCOPE(buffersScopeId);

        cgltf_buffer const& gltfBuffer = model.buffers[i];

        assert(gltfBuffer.data);
        assert(gltfBuffer.size > 0);

        nstl::span<unsigned char const> data{ static_cast<unsigned char const*>(gltfBuffer.data), gltfBuffer.size };
        size_t bufferSize = sizeof(data[0]) * data.size();

        vkgfx::BufferMetadata metadata{
            .usage = vkgfx::BufferUsage::VertexIndexBuffer,
            .location = vkgfx::BufferLocation::DeviceLocal,
            .isMutable = false,
        };
        auto handle = resourceManager.createBuffer(bufferSize, nstl::move(metadata)); // TODO split buffer into several different parts
        resourceManager.uploadBuffer(handle, data);
        m_gltfResources->buffers.push_back(handle);
    }

    m_gltfResources->samplers.reserve(model.samplers_count);
    for (size_t i = 0; i < model.samplers_count; i++)
    {
        MEMORY_TRACKING_SCOPE(samplersScopeId);

        cgltf_sampler const& gltfSampler = model.samplers[i];

        auto convertFilterMode = [](int gltfMode)
        {
            // TODO remove magic numbers

            switch (gltfMode)
            {
            case 9728: // NEAREST:
            case 9984: // NEAREST_MIPMAP_NEAREST:
            case 9986: // NEAREST_MIPMAP_LINEAR:
                return vko::SamplerFilterMode::Nearest;

            case 0:
            case 9729: // LINEAR:
            case 9985: // LINEAR_MIPMAP_NEAREST:
            case 9987: // LINEAR_MIPMAP_LINEAR:
                return vko::SamplerFilterMode::Linear;
            }

            assert(false);
            return vko::SamplerFilterMode::Nearest;
        };

        auto convertWrapMode = [](int gltfMode)
        {
            // TODO remove magic numbers

            switch (gltfMode)
            {
            case 10497: // REPEAT:
                return vko::SamplerWrapMode::Repeat;
            case 33071: // CLAMP_TO_EDGE:
                return vko::SamplerWrapMode::ClampToEdge;
            case 33648: // MIRRORED_REPEAT:
                return vko::SamplerWrapMode::Mirror;
            }

            assert(false);
            return vko::SamplerWrapMode::Repeat;
        };

        auto magFilter = convertFilterMode(gltfSampler.mag_filter);
        auto minFilter = convertFilterMode(gltfSampler.min_filter);
        auto wrapU = convertWrapMode(gltfSampler.wrap_s);
        auto wrapV = convertWrapMode(gltfSampler.wrap_t);

        auto handle = resourceManager.createSampler(magFilter, minFilter, wrapU, wrapV);
        m_gltfResources->samplers.push_back(handle);
    }

    m_gltfResources->images.reserve(model.images_count);
    for (size_t i = 0; i < model.images_count; i++)
    {
        MEMORY_TRACKING_SCOPE(imagesScopeId);

        cgltf_image const& gltfImage = model.images[i];
        assert(gltfImage.uri && !gltfImage.buffer_view);

        nstl::string imagePath = basePath + gltfImage.uri;

        // TODO leads to unnecessary data copy; change that
        nstl::optional<ImageData> imageData = loadImage(vkc::utils::readBinaryFile(imagePath));
        assert(imageData);

        assert(!imageData->bytes.empty());
        assert(!imageData->mips.empty());
        assert(imageData->mips[0].size > 0);

        vkgfx::ImageMetadata metadata;
        metadata.width = imageData->width;
        metadata.height = imageData->height;

        // TODO support all mips

        ImageData::MipData const& mipData = imageData->mips[0];
        auto bytes = nstl::span<unsigned char const>{ imageData->bytes }.subspan(mipData.offset, mipData.size);

        metadata.byteSize = bytes.size();
        metadata.format = imageData->format;

        auto handle = resourceManager.createImage(metadata);
        resourceManager.uploadImage(handle, bytes);
        m_gltfResources->images.push_back(handle);
    }

    m_gltfResources->textures.reserve(model.textures_count);
    for (size_t i = 0; i < model.textures_count; i++)
    {
        MEMORY_TRACKING_SCOPE(texturesScopeId);

        cgltf_texture const& gltfTexture = model.textures[i];

        size_t imageIndex = findIndex(gltfTexture.image, model.images, model.images_count);

        vkgfx::ImageHandle imageHandle = m_gltfResources->images[imageIndex];
        
        assert(imageHandle);

        vkgfx::Texture texture;
        texture.image = imageHandle;
        texture.sampler = m_defaultSampler;
        if (gltfTexture.sampler)
        {
            size_t samplerIndex = findIndex(gltfTexture.sampler, model.samplers, model.samplers_count);
            texture.sampler = m_gltfResources->samplers[samplerIndex];
        }

        vkgfx::TextureHandle handle = resourceManager.createTexture(nstl::move(texture));
        m_gltfResources->textures.push_back(handle);
    }

    m_gltfResources->materials.reserve(model.materials_count);
    for (size_t i = 0; i < model.materials_count; i++)
    {
        MEMORY_TRACKING_SCOPE(materialsScopeId);

        cgltf_material const& gltfMaterial = model.materials[i];

        cgltf_pbr_metallic_roughness const& gltfRoughness = gltfMaterial.pbr_metallic_roughness;

        vkgfx::Material material;
        material.albedo = m_defaultAlbedoTexture;
        material.normalMap = m_defaultNormalMapTexture;

        struct MaterialUniformBuffer
        {
            tglm::vec4 color;
        };

        MaterialUniformBuffer values;
        values.color = createColor(gltfRoughness.base_color_factor);

        vkgfx::BufferMetadata metadata{
            .usage = vkgfx::BufferUsage::UniformBuffer,
            .location = vkgfx::BufferLocation::HostVisible,
            .isMutable = false,
        };
        auto buffer = resourceManager.createBuffer(sizeof(MaterialUniformBuffer), nstl::move(metadata));
        resourceManager.uploadBuffer(buffer, nstl::blob_view{ &values, sizeof(MaterialUniformBuffer) });
        m_gltfResources->additionalBuffers.push_back(buffer);

        material.uniformBuffer = buffer;

        if (gltfRoughness.base_color_texture.texture)
        {
            size_t textureIndex = findIndex(gltfRoughness.base_color_texture.texture, model.textures, model.textures_count);
            auto textureHandle = m_gltfResources->textures[textureIndex];
            if (textureHandle)
                material.albedo = textureHandle;
        }

        if (gltfMaterial.normal_texture.texture)
        {
            size_t textureIndex = findIndex(gltfMaterial.normal_texture.texture, model.textures, model.textures_count);
            auto textureHandle = m_gltfResources->textures[textureIndex];
            if (textureHandle)
                material.normalMap = textureHandle;
        }

        DemoMaterial& demoMaterial = m_gltfResources->materials.emplace_back();

        demoMaterial.handle = resourceManager.createMaterial(nstl::move(material));

        demoMaterial.metadata.renderConfig.wireframe = false;
        demoMaterial.metadata.renderConfig.cullBackfaces = !gltfMaterial.double_sided;
        demoMaterial.metadata.uniformConfig.hasBuffer = true;
        demoMaterial.metadata.uniformConfig.hasAlbedoTexture = true;
        demoMaterial.metadata.uniformConfig.hasNormalMap = true;
    }

    m_gltfResources->meshes.reserve(model.meshes_count);
    for (size_t meshIndex = 0; meshIndex < model.meshes_count; meshIndex++)
    {
        MEMORY_TRACKING_SCOPE(meshesScopeId);

        cgltf_mesh const& gltfMesh = model.meshes[meshIndex];

        DemoMesh& demoMesh = m_gltfResources->meshes.emplace_back();
        demoMesh.primitives.reserve(gltfMesh.primitives_count);
        for (size_t primitiveIndex = 0; primitiveIndex < gltfMesh.primitives_count; primitiveIndex++)
        {
            cgltf_primitive const& gltfPrimitive = gltfMesh.primitives[primitiveIndex];

            DemoPrimitive& demoPrimitive = demoMesh.primitives.emplace_back();

            vkgfx::Mesh mesh;

            {
                auto findIndexType = [](int gltfComponentType)
                {
                    switch (gltfComponentType)
                    {
                    case cgltf_component_type_r_8u:
                        return vkgfx::IndexType::UnsignedByte;
                    case cgltf_component_type_r_16u:
                        return vkgfx::IndexType::UnsignedShort;
                    case cgltf_component_type_r_32u:
                        return vkgfx::IndexType::UnsignedInt;
                    }

                    assert(false);
                    return vkgfx::IndexType::UnsignedShort;
                };

                cgltf_accessor const* gltfAccessor = gltfPrimitive.indices;
                assert(gltfAccessor);
                cgltf_buffer_view const* gltfBufferView = gltfAccessor->buffer_view;
                assert(gltfBufferView);

                mesh.indexBuffer.buffer = m_gltfResources->buffers[findIndex(gltfBufferView->buffer, model.buffers, model.buffers_count)];
                mesh.indexBuffer.offset = gltfBufferView->offset + gltfAccessor->offset;
                mesh.indexCount = gltfAccessor->count;
                mesh.indexType = findIndexType(gltfAccessor->component_type);
            }

            mesh.vertexBuffers.reserve(gltfPrimitive.attributes_count);
            demoPrimitive.metadata.vertexConfig.bindings.reserve(gltfPrimitive.attributes_count);
            demoPrimitive.metadata.vertexConfig.attributes.reserve(gltfPrimitive.attributes_count);

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

                cgltf_accessor const* gltfAccessor = gltfAttribute.data;
                assert(gltfAccessor);
                cgltf_buffer_view const* gltfBufferView = gltfAccessor->buffer_view;
                assert(gltfBufferView);

                size_t bufferIndex = findIndex(gltfBufferView->buffer, model.buffers, model.buffers_count);

                vkgfx::BufferWithOffset& attributeBuffer = mesh.vertexBuffers.emplace_back();
                attributeBuffer.buffer = m_gltfResources->buffers[bufferIndex];
                attributeBuffer.offset = gltfBufferView->offset + gltfAccessor->offset; // TODO can be improved

                if (name == "COLOR_0")
                    demoPrimitive.metadata.attributeSemanticsConfig.hasColor = true;
                if (name == "TEXCOORD_0")
                    demoPrimitive.metadata.attributeSemanticsConfig.hasUv = true;
                if (name == "NORMAL")
                    demoPrimitive.metadata.attributeSemanticsConfig.hasNormal = true;
                if (name == "TANGENT")
                    demoPrimitive.metadata.attributeSemanticsConfig.hasTangent = true;

                vkgfx::AttributeType attributeType = findAttributeType(gltfAccessor->type, gltfAccessor->component_type);

                size_t stride = gltfBufferView->stride;
                if (stride == 0)
                    stride = getAttributeByteSize(attributeType);

                vkgfx::VertexConfiguration::Binding& bindingConfig = demoPrimitive.metadata.vertexConfig.bindings.emplace_back();
                bindingConfig.stride = stride;

                vkgfx::VertexConfiguration::Attribute& attributeConfig = demoPrimitive.metadata.vertexConfig.attributes.emplace_back();
                attributeConfig.binding = attributeIndex;
                attributeConfig.location = *location;
                attributeConfig.offset = 0; // TODO can be improved
                attributeConfig.type = attributeType;
            }

            // TODO implement
            assert(gltfPrimitive.type == cgltf_primitive_type_triangles);
            demoPrimitive.metadata.vertexConfig.topology = vkgfx::VertexTopology::Triangles;

            demoPrimitive.metadata.materialIndex = findIndex(gltfPrimitive.material, model.materials, model.materials_count); // TODO check

            demoPrimitive.handle = resourceManager.createMesh(nstl::move(mesh));
        }
    }

    m_gltfResources->cameraParameters.reserve(model.cameras_count);
    for (size_t i = 0; i < model.cameras_count; i++)
    {
        MEMORY_TRACKING_SCOPE(camerasScopeId);

        cgltf_camera const& gltfCamera = model.cameras[i];

        if (gltfCamera.type == cgltf_camera_type_perspective)
        {
            cgltf_camera_perspective const& gltfParams = gltfCamera.data.perspective;

            assert(gltfParams.has_zfar);

            m_gltfResources->cameraParameters.push_back(vkgfx::TestCameraParameters{
                .fov = tglm::degrees(static_cast<float>(gltfParams.yfov)),
                .nearZ = static_cast<float>(gltfParams.znear),
                .farZ = static_cast<float>(gltfParams.zfar),
            });
        }
        else
        {
            assert(false);
        }
    }

    {
        assert(model.scene);
        m_demoScene = createDemoScene(model, *model.scene);

        nstl::simple_sort(m_demoScene.objects.begin(), m_demoScene.objects.end(), [](vkgfx::TestObject const& lhs, vkgfx::TestObject const& rhs)
        {
            if (lhs.pipeline != rhs.pipeline)
                return lhs.pipeline < rhs.pipeline;

            return lhs.material < rhs.material;
        });

        if (!m_demoScene.cameras.empty())
        {
            DemoCamera const& camera = m_demoScene.cameras[0];
            m_cameraTransform = camera.transform;
            m_cameraParameters = m_gltfResources->cameraParameters[camera.parametersIndex];
        }
    }

    return true;
}

bool DemoApplication::editorLoadScene(editor::assets::Uuid sceneId)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor");
    static auto shadersScopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Shader");

    MEMORY_TRACKING_SCOPE(scopeId);

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    m_editorGltfResources = nstl::make_unique<EditorGltfResources>();

    // TODO move somewhere else? Doesn't seem to be related to the GLTF model
    // TODO only load actually used permutations
    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultVertexShader->getAll())
    for (auto const& pair : m_defaultVertexShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Vertex, "main");
        m_editorGltfResources->shaderModules.insert_or_assign(modulePath, handle);
        m_editorGltfResources->newShaderModules.insert_or_assign(modulePath, m_newRenderer->create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::vertex,
        }));
    }

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultVertexShader->getAll())
    for (auto const& pair : m_newDefaultVertexShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Vertex, "main");
        m_editorGltfResources->shaderModules.insert_or_assign(modulePath, handle);
        m_editorGltfResources->newShaderModules.insert_or_assign(modulePath, m_newRenderer->create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::vertex,
        }));
    }

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultFragmentShader->getAll())
    for (auto const& pair : m_defaultFragmentShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
//         auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Fragment, "main");
//         m_editorGltfResources->shaderModules.insert_or_assign(modulePath, handle);
        m_editorGltfResources->newShaderModules.insert_or_assign(modulePath, m_newRenderer->create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::fragment,
        }));
    }

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultFragmentShader->getAll())
    for (auto const& pair : m_newDefaultFragmentShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Fragment, "main");
        m_editorGltfResources->shaderModules.insert_or_assign(modulePath, handle);
        m_editorGltfResources->newShaderModules.insert_or_assign(modulePath, m_newRenderer->create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::fragment,
        }));
    }

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_shadowmapVertexShader->getAll())
    for (auto const& pair : m_shadowmapVertexShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Vertex, "main");
        m_editorGltfResources->shaderModules.insert_or_assign(modulePath, handle);
        m_editorGltfResources->newShaderModules.insert_or_assign(modulePath, m_newRenderer->create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::vertex,
        }));
    }

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_shadowmapVertexShader->getAll())
    for (auto const& pair : m_newShadowmapVertexShader->getAll())
    {
        MEMORY_TRACKING_SCOPE(shadersScopeId);

        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readBinaryFile(modulePath), vko::ShaderModuleType::Vertex, "main");
        m_editorGltfResources->shaderModules.insert_or_assign(modulePath, handle);
        m_editorGltfResources->newShaderModules.insert_or_assign(modulePath, m_newRenderer->create_shader({
            .filename = modulePath,
            .stage = gfx::shader_stage::vertex,
        }));
    }

    editor::assets::SceneData scene = m_assetDatabase->loadScene(sceneId);

    for (editor::assets::ObjectDescription const& object : scene.objects)
    {
        if (object.mesh)
        {
            editor::assets::Uuid id = object.mesh->id;
            if (m_editorGltfResources->meshes.find(id) == m_editorGltfResources->meshes.end())
                editorLoadMesh(id);

            for (DemoPrimitive const& primitive : m_editorGltfResources->meshes[id].primitives)
            {
                DemoMaterial const& material = m_editorGltfResources->materials[primitive.metadata.materialUuid];

                vkgfx::PipelineKey pipelineKey;
                pipelineKey.renderConfig = material.metadata.renderConfig;

                // TODO think how to handle multiple descriptor set layouts properly
                pipelineKey.uniformConfigs = {
                    // TODO get shared frame uniform config from the Renderer
                    vkgfx::UniformConfiguration{
                        .hasBuffer = true,
                        .hasAlbedoTexture = false,
                        .hasNormalMap = false,
                        .hasShadowMap = true,
                    },
                    material.metadata.uniformConfig,
                    vkgfx::UniformConfiguration{
                        .hasBuffer = true,
                        .hasAlbedoTexture = false,
                        .hasNormalMap = false,
                    },
                };

                pipelineKey.vertexConfig = primitive.metadata.vertexConfig;

                struct DemoObjectPushConstants
                {
                    tglm::mat4 model;
                };

                pipelineKey.pushConstantRanges = { vkgfx::PushConstantRange{.offset = 0, .size = sizeof(DemoObjectPushConstants), } };

                // TODO reimplement
                ShaderConfiguration shaderConfiguration;
                shaderConfiguration.hasTexture = material.metadata.uniformConfig.hasAlbedoTexture;
                shaderConfiguration.hasNormalMap = material.metadata.uniformConfig.hasNormalMap;
                shaderConfiguration.hasColor = primitive.metadata.attributeSemanticsConfig.hasColor;
                shaderConfiguration.hasTexCoord = primitive.metadata.attributeSemanticsConfig.hasUv;
                shaderConfiguration.hasNormal = primitive.metadata.attributeSemanticsConfig.hasNormal;
                shaderConfiguration.hasTangent = primitive.metadata.attributeSemanticsConfig.hasTangent;

                nstl::string const* vertexShaderPath = m_defaultVertexShader->get(shaderConfiguration);
                nstl::string const* newVertexShaderPath = m_newDefaultVertexShader->get(shaderConfiguration);
                nstl::string const* fragmentShaderPath = m_defaultFragmentShader->get(shaderConfiguration);
                nstl::string const* newFragmentShaderPath = m_newDefaultFragmentShader->get(shaderConfiguration);

                assert(vertexShaderPath && fragmentShaderPath);
                assert(newVertexShaderPath && newFragmentShaderPath);

                vkgfx::ShaderModuleHandle vertexShaderModule = m_editorGltfResources->shaderModules[*vertexShaderPath];
                vkgfx::ShaderModuleHandle fragmentShaderModule = m_editorGltfResources->shaderModules[*fragmentShaderPath];

                pipelineKey.shaderHandles = { vertexShaderModule, fragmentShaderModule };

                vkgfx::PipelineHandle pipeline = resourceManager.getOrCreatePipeline(pipelineKey);

                gfx::shader_handle vertexShader = m_editorGltfResources->newShaderModules[*newVertexShaderPath];
                gfx::shader_handle fragmentShader = m_editorGltfResources->newShaderModules[*newFragmentShaderPath];
                m_renderstates.push_back(m_newRenderer->create_renderstate({
                    .shaders = nstl::array{ vertexShader, fragmentShader },
                    .renderpass = m_newRenderer->get_main_renderpass(),
                    .vertex_config = primitive.metadata.newVertexConfig,
                    .descriptorgroup_layouts = nstl::array{
                        gfx::descriptorgroup_layout_view {
                            .entries = nstl::array{
                                gfx::descriptor_layout_entry{0, gfx::descriptor_type::buffer},
                                gfx::descriptor_layout_entry{1, gfx::descriptor_type::buffer},
//                                 gfx::descriptor_layout_entry{3, gfx::descriptor_type::combined_image_sampler}, // TODO enable
                            },
                        },
                        material.metadata.newUniformConfig,
                        gfx::descriptorgroup_layout_view {
                            .entries = nstl::array{
                                gfx::descriptor_layout_entry{0, gfx::descriptor_type::buffer},
                            },
                        },
                    },
                    .flags = material.metadata.newRenderConfig,
                }));

                gfx::renderstate_handle state = m_renderstates.back();

                nstl::string const* shadowmapVertexShaderPath = m_shadowmapVertexShader->get({});
                nstl::string const* newShadowmapVertexShaderPath = m_newShadowmapVertexShader->get({});
                assert(shadowmapVertexShaderPath);
                assert(newShadowmapVertexShaderPath);
                vkgfx::ShaderModuleHandle shadowmapVertexShaderModule = m_editorGltfResources->shaderModules[*shadowmapVertexShaderPath];

                vkgfx::PipelineKey shadowmapPipelineKey = {
                    .shaderHandles = {shadowmapVertexShaderModule},
                    .uniformConfigs = {
                        vkgfx::UniformConfiguration{
                            .hasBuffer = true,
                            .hasAlbedoTexture = false,
                            .hasNormalMap = false,
                        },
                    },
                    .vertexConfig = primitive.metadata.vertexConfig,
                    .renderConfig = {
                        .depthBias = true,
                    },
                    .pushConstantRanges = { vkgfx::PushConstantRange{.offset = 0, .size = sizeof(DemoObjectPushConstants), } },
                    .isShadowmap = true,
                };

                vkgfx::PipelineHandle shadowmapPipeline = resourceManager.getOrCreatePipeline(shadowmapPipelineKey);

                gfx::shader_handle shadowmapVertexShader = m_editorGltfResources->newShaderModules[*newShadowmapVertexShaderPath];
                m_renderstates.push_back(m_newRenderer->create_renderstate({
                    .shaders = nstl::array{ shadowmapVertexShader },
                    .renderpass = m_shadowRenderpass,
                    .vertex_config = primitive.metadata.newVertexConfig,
                    .descriptorgroup_layouts = nstl::array{
                        gfx::descriptorgroup_layout_view {
                            .entries = nstl::array{
                                gfx::descriptor_layout_entry{0, gfx::descriptor_type::buffer},
                            },
                        },
                        gfx::descriptorgroup_layout_view {
                            .entries = {},
                        },
                        gfx::descriptorgroup_layout_view {
                            .entries = nstl::array{
                                gfx::descriptor_layout_entry{0, gfx::descriptor_type::buffer},
                            },
                        },
                    },
                    .flags = {
                        .depth_bias = true,
                    },
                }));

                gfx::renderstate_handle shadowmapState = m_renderstates.back();

                struct DemoObjectUniformBuffer
                {
                    tglm::vec4 color;
                };

                vkgfx::BufferMetadata uniformBufferMetadata{
                    .usage = vkgfx::BufferUsage::UniformBuffer,
                    .location = vkgfx::BufferLocation::HostVisible,
                    .isMutable = false,
                };
                vkgfx::BufferHandle uniformBuffer = resourceManager.createBuffer(sizeof(DemoObjectUniformBuffer), nstl::move(uniformBufferMetadata));
                m_editorGltfResources->additionalBuffers.push_back(uniformBuffer);

                DemoObjectUniformBuffer uniformValues;
                uniformValues.color = { 1.0f, 1.0f, 1.0f, 0.0f };
                resourceManager.uploadBuffer(uniformBuffer, nstl::blob_view{ &uniformValues, sizeof(uniformValues) });

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

                struct DemoObjectUniformBuffer
                {
                    tglm::mat4 model;
                    tglm::vec4 color;
                };

                auto objectUniformBuffer = m_newRenderer->create_buffer({
                    .size = sizeof(DemoObjectUniformBuffer),
                    .usage = gfx::buffer_usage::uniform,
                    .location = gfx::buffer_location::host_visible,
                    .is_mutable = false,
                });

                {
                    DemoObjectUniformBuffer values = {
                        .model = calculateTransform(),
                        .color = { 1.0f, 1.0f, 1.0f, 0.0f },
                    };

                    m_newRenderer->buffer_upload_sync(objectUniformBuffer, { &values, sizeof(values) });
                }

                auto descriptorGroup = m_newRenderer->create_descriptorgroup({
                    .entries = nstl::array{ gfx::descriptorgroup_entry{ 0, objectUniformBuffer } },
                });

                DemoObjectPushConstants pushConstants;
                pushConstants.model = calculateTransform();

                vkgfx::TestObject& testObject = m_demoScene.objects.emplace_back();
                testObject.mesh = primitive.handle;
                testObject.material = material.handle;
                testObject.materialDescriptorGroup = material.descriptorgroup;
                testObject.newUniformBuffer = objectUniformBuffer;
                testObject.descriptorGroup = descriptorGroup;
                testObject.pipeline = pipeline;
                testObject.shadowmapPipeline = shadowmapPipeline;
                testObject.state = state;
                testObject.shadowmapState = shadowmapState;
                testObject.uniformBuffer = uniformBuffer;
                testObject.pushConstants.resize(sizeof(pushConstants));
                memcpy(testObject.pushConstants.data(), &pushConstants, testObject.pushConstants.size());

                testObject.indexBuffer = primitive.indexBuffer;
                testObject.vertexBuffers = primitive.vertexBuffers;
                testObject.indexType = primitive.indexType;
                testObject.indexCount = primitive.indexCount;
            }
        }
    }

    return true;
}

void DemoApplication::editorLoadImage(editor::assets::Uuid id)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Image");
    MEMORY_TRACKING_SCOPE(scopeId);

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    // TODO leads to unnecessary data copy; change that
    nstl::optional<ImageData> imageData = loadImage(m_assetDatabase->loadImage(id));
    assert(imageData);

    assert(!imageData->bytes.empty());
    assert(!imageData->mips.empty());
    assert(imageData->mips[0].size > 0);

    vkgfx::ImageMetadata metadata;
    metadata.width = imageData->width;
    metadata.height = imageData->height;

    // TODO support all mips

    ImageData::MipData const& mipData = imageData->mips[0];
    auto bytes = nstl::span<unsigned char const>{ imageData->bytes }.subspan(mipData.offset, mipData.size);

    metadata.byteSize = bytes.size();
    metadata.format = imageData->format;

    auto handle = resourceManager.createImage(metadata);
    resourceManager.uploadImage(handle, bytes);
    m_editorGltfResources->images[id] = handle;

    auto image = m_newRenderer->create_image({
        .width = imageData->width,
        .height = imageData->height,
        .format = static_cast<gfx::image_format>(static_cast<size_t>(imageData->format)), // TODO: remove
        .type = gfx::image_type::color,
        .usage = gfx::image_usage::upload_sampled,
    });
    m_newRenderer->image_upload_sync(image, bytes);
    m_editorGltfResources->newImages[id] = nstl::move(image);
}

void DemoApplication::editorLoadMaterial(editor::assets::Uuid id)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Material");

    MEMORY_TRACKING_SCOPE(scopeId);

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    editor::assets::MaterialData materialData = m_assetDatabase->loadMaterial(id);

    vkgfx::Material material;
    material.albedo = m_defaultAlbedoTexture;
    material.normalMap = m_defaultNormalMapTexture;

    struct MaterialUniformBuffer
    {
        tglm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    MaterialUniformBuffer values;
    if (materialData.baseColor)
        values.color = tglm::vec4{ materialData.baseColor->data };

    vkgfx::BufferMetadata metadata{
        .usage = vkgfx::BufferUsage::UniformBuffer,
        .location = vkgfx::BufferLocation::HostVisible,
        .isMutable = false,
    };
    auto buffer = resourceManager.createBuffer(sizeof(values), nstl::move(metadata));
    resourceManager.uploadBuffer(buffer, { &values, sizeof(values) });
    m_editorGltfResources->additionalBuffers.push_back(buffer);

    material.uniformBuffer = buffer;

    auto newBuffer = m_newRenderer->create_buffer({
        .size = sizeof(values),
        .usage = gfx::buffer_usage::uniform,
        .location = gfx::buffer_location::host_visible,
        .is_mutable = false,
    });

    m_newRenderer->buffer_upload_sync(newBuffer, { &values, sizeof(values) });

    nstl::vector<gfx::descriptorgroup_entry> descriptor_entries;
    nstl::vector<gfx::descriptor_layout_entry> descriptor_layout_entries;
    descriptor_entries.push_back({ 0, newBuffer });
    descriptor_layout_entries.push_back({ 0, gfx::descriptor_type::buffer });

    if (materialData.baseColorTexture)
    {
        editor::assets::Uuid imageId = materialData.baseColorTexture->image;
        if (m_editorGltfResources->images.find(imageId) == m_editorGltfResources->images.end())
            editorLoadImage(imageId);

        auto textureHandle = resourceManager.createTexture(vkgfx::Texture{
            .image = m_editorGltfResources->images[imageId],
            .sampler = m_defaultSampler, // TODO create actual sampler
        });

        material.albedo = textureHandle;

        // TODO create actual sampler
        descriptor_entries.push_back({ 1, {m_editorGltfResources->newImages[imageId], m_newDefaultSampler} });
        descriptor_layout_entries.push_back({ 1, gfx::descriptor_type::combined_image_sampler });
    }

    if (materialData.normalTexture)
    {
        editor::assets::Uuid imageId = materialData.normalTexture->image;
        if (m_editorGltfResources->images.find(imageId) == m_editorGltfResources->images.end())
            editorLoadImage(imageId);

        auto textureHandle = resourceManager.createTexture(vkgfx::Texture{
            .image = m_editorGltfResources->images[imageId],
            .sampler = m_defaultSampler, // TODO create actual sampler
            });

        material.normalMap = textureHandle;

        // TODO create actual sampler
        descriptor_entries.push_back({ 2, {m_editorGltfResources->newImages[imageId], m_newDefaultSampler} });
        descriptor_layout_entries.push_back({ 2, gfx::descriptor_type::combined_image_sampler });
    }

    DemoMaterial& demoMaterial = m_editorGltfResources->materials[id];

    demoMaterial.handle = resourceManager.createMaterial(nstl::move(material));

    demoMaterial.buffer = newBuffer;
    demoMaterial.descriptorgroup = m_newRenderer->create_descriptorgroup({
        .entries = descriptor_entries,
    });

    demoMaterial.metadata.renderConfig.wireframe = false;
    demoMaterial.metadata.renderConfig.cullBackfaces = !materialData.doubleSided;
    demoMaterial.metadata.uniformConfig.hasBuffer = true;
    demoMaterial.metadata.uniformConfig.hasAlbedoTexture = true;
    demoMaterial.metadata.uniformConfig.hasNormalMap = true;

    demoMaterial.metadata.newRenderConfig.wireframe = false;
    demoMaterial.metadata.newRenderConfig.cull_backfaces = !materialData.doubleSided;
    demoMaterial.metadata.newUniformConfig = { descriptor_layout_entries };
}

void DemoApplication::editorLoadMesh(editor::assets::Uuid id)
{
    static auto scopeId = memory::tracking::create_scope_id("Scene/Load/Editor/Mesh");

    MEMORY_TRACKING_SCOPE(scopeId);

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    // Mesh buffer
    nstl::blob data = m_assetDatabase->loadMeshData(id);

    vkgfx::BufferMetadata metadata{
        .usage = vkgfx::BufferUsage::VertexIndexBuffer,
        .location = vkgfx::BufferLocation::DeviceLocal,
        .isMutable = false,
    };
    vkgfx::BufferHandle meshBufferHandle = resourceManager.createBuffer(data.size(), nstl::move(metadata));
    resourceManager.uploadBuffer(meshBufferHandle, data);
    m_editorGltfResources->meshBuffers[id] = meshBufferHandle;

    auto meshBuffer = m_newRenderer->create_buffer({
        .size = data.size(),
        .usage = gfx::buffer_usage::vertex_index,
        .location = gfx::buffer_location::device_local,
        .is_mutable = false,
    });
    m_newRenderer->buffer_upload_sync(meshBuffer, data);
    m_editorGltfResources->newMeshBuffers[id] = meshBuffer;

    editor::assets::MeshData meshData = m_assetDatabase->loadMesh(id);

    DemoMesh& demoMesh = m_editorGltfResources->meshes[id];
    
    for (editor::assets::PrimitiveDescription const& primitiveData : meshData.primitives)
    {
        editor::assets::Uuid materialId = primitiveData.material;
        if (m_editorGltfResources->materials.find(materialId) == m_editorGltfResources->materials.end())
            editorLoadMaterial(materialId);

        DemoPrimitive& demoPrimitive = demoMesh.primitives.emplace_back();

        vkgfx::Mesh mesh;

        {
            auto findIndexType = [](editor::assets::DataComponentType componentType)
            {
                switch (componentType)
                {
                case editor::assets::DataComponentType::Int8:
                    return vkgfx::IndexType::UnsignedByte;
                case editor::assets::DataComponentType::UInt16:
                    return vkgfx::IndexType::UnsignedShort;
                case editor::assets::DataComponentType::UInt32:
                    return vkgfx::IndexType::UnsignedInt;
                default:
                    assert(false);
                }

                assert(false);
                return vkgfx::IndexType::UnsignedShort;
            };
            auto newFindIndexType = [](editor::assets::DataComponentType componentType)
            {
                switch (componentType)
                {
                case editor::assets::DataComponentType::Int8:
                    assert(false);
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

            assert(primitiveData.indices.type == editor::assets::DataType::Scalar);

            mesh.indexBuffer.buffer = m_editorGltfResources->meshBuffers[id];
            mesh.indexBuffer.offset = primitiveData.indices.bufferOffset;
            mesh.indexCount = primitiveData.indices.count;
            mesh.indexType = findIndexType(primitiveData.indices.componentType);
            // TODO use stride?

            demoPrimitive.indexBuffer = { meshBuffer, primitiveData.indices.bufferOffset };
            demoPrimitive.indexType = newFindIndexType(primitiveData.indices.componentType);
            demoPrimitive.indexCount = primitiveData.indices.count;
        }

        for (editor::assets::VertexAttributeDescription const& attributeData : primitiveData.vertexAttributes)
        {
            nstl::optional<size_t> location = findAttributeLocation(attributeData.semantic);

            if (!location)
            {
                logging::warn("Skipping attribute");
                continue;
            }

            demoPrimitive.vertexBuffers.push_back({ meshBuffer, attributeData.accessor.bufferOffset });

            vkgfx::BufferWithOffset& attributeBuffer = mesh.vertexBuffers.emplace_back();
            attributeBuffer.buffer = meshBufferHandle;
            attributeBuffer.offset = attributeData.accessor.bufferOffset;

            if (attributeData.semantic == editor::assets::AttributeSemantic::Color)
                demoPrimitive.metadata.attributeSemanticsConfig.hasColor = true;
            if (attributeData.semantic == editor::assets::AttributeSemantic::Texcoord)
                demoPrimitive.metadata.attributeSemanticsConfig.hasUv = true;
            if (attributeData.semantic == editor::assets::AttributeSemantic::Normal)
                demoPrimitive.metadata.attributeSemanticsConfig.hasNormal = true;
            if (attributeData.semantic == editor::assets::AttributeSemantic::Tangent)
                demoPrimitive.metadata.attributeSemanticsConfig.hasTangent = true;

            vkgfx::AttributeType attributeType = findAttributeType(attributeData.accessor.type, attributeData.accessor.componentType);
            gfx::attribute_type newAttributeType = newFindAttributeType(attributeData.accessor.type, attributeData.accessor.componentType);

            vkgfx::VertexConfiguration::Binding& bindingConfig = demoPrimitive.metadata.vertexConfig.bindings.emplace_back();
            bindingConfig.stride = attributeData.accessor.stride;

            size_t attributeIndex = demoPrimitive.metadata.vertexConfig.attributes.size(); // TODO check
            vkgfx::VertexConfiguration::Attribute& attributeConfig = demoPrimitive.metadata.vertexConfig.attributes.emplace_back();
            attributeConfig.binding = attributeIndex;
            attributeConfig.location = *location;
            attributeConfig.offset = 0; // TODO can be improved
            attributeConfig.type = attributeType;

            // TODO probably a single buffer can be used
            size_t buffer_index = demoPrimitive.metadata.newVertexConfig.buffer_bindings.size();
            demoPrimitive.metadata.newVertexConfig.buffer_bindings.push_back({
                .buffer_index = buffer_index,
                .stride = attributeData.accessor.stride,
            });

            size_t binding_index = buffer_index;
            demoPrimitive.metadata.newVertexConfig.attributes.push_back({
                .location = *location,
                .buffer_binding_index = binding_index,
                .offset = 0,
                .type = newAttributeType,
            });
        }

        // TODO implement
        assert(primitiveData.topology == editor::assets::Topology::Triangles);
        demoPrimitive.metadata.vertexConfig.topology = vkgfx::VertexTopology::Triangles;
        demoPrimitive.metadata.newVertexConfig.topology = gfx::vertex_topology::triangles;

        demoPrimitive.metadata.materialUuid = materialId;

        demoPrimitive.handle = resourceManager.createMesh(nstl::move(mesh));
    }
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
        ImGui::Text("Uptime %.3f", m_appTime.getTime());
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
    update();
    draw();

    drawTest();

    m_renderer->setCameraTransform(m_cameraTransform);
    m_renderer->setCameraParameters(m_cameraParameters);
    m_renderer->setLightParameters(m_lightParameters);

    for (auto const& demoObject : m_demoScene.objects)
        m_renderer->addOneFrameTestObject(demoObject);
    m_services.debugDraw().queueGeometry(*m_renderer);
    m_imGuiDrawer->queueGeometry(*m_renderer);

    m_renderer->draw();

    m_fpsDrawnFrames++;
}

void DemoApplication::update()
{
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
    m_newRenderer->wait_for_next_frame();
    m_newRenderer->begin_frame();

    m_newRenderer->renderpass_begin({
        .renderpass = m_newRenderer->get_main_renderpass(),
        .framebuffer = m_newRenderer->acquire_main_framebuffer(),
    });

    for (vkgfx::TestObject const& object : m_demoScene.objects)
    {
        m_newRenderer->draw_indexed({
            .renderstate = object.state,
            .descriptorgroups = nstl::array{ m_cameraDescriptorGroup, object.materialDescriptorGroup, object.descriptorGroup },

            .vertex_buffers = object.vertexBuffers,
            .index_buffer = object.indexBuffer,
            .index_type = object.indexType,

            .index_count = object.indexCount,
            .first_index = 0,
            .vertex_offset = 0,
        });
    }

    m_newRenderer->renderpass_end();

    m_newRenderer->submit();
}

//////////////////////////////////////////////////////////////////////////

namespace
{
    struct Vertex
    {
        tglm::vec3 position;
        tglm::vec3 color;
    };

    using IndexT = uint16_t;

    struct FrameUniformBuffer
    {
        tglm::mat4 view = tglm::mat4::identity();
        tglm::mat4 projection = tglm::mat4::identity();
    };

    struct MaterialUniformBuffer
    {
        tglm::vec4 color = { 1, 1, 1, 1 };
    };

    struct ObjectUniformBuffer
    {
        tglm::mat4 model = tglm::mat4::identity();
        tglm::vec4 color = { 1, 1, 1, 1 };
    };

    struct TestResources
    {
        gfx::buffer_handle vertexBuffer;
        gfx::buffer_handle indexBuffer;

        gfx::buffer_handle frameBuffer;
        gfx::descriptorgroup_handle frameDescriptors;
        gfx::buffer_handle materialBuffer;
        gfx::descriptorgroup_handle materialDescriptors;
        gfx::buffer_handle objectBuffer;
        gfx::descriptorgroup_handle objectDescriptors;

        gfx::shader_handle vertexShader;
        gfx::shader_handle fragmentShader;
        gfx::renderstate_handle renderstate;
    };

    TestResources testResources;
}

void DemoApplication::createTestResources()
{
    auto createTestBuffer = [this](gfx::buffer_usage usage, nstl::blob_view data)
    {
        gfx::buffer_handle buffer = m_newRenderer->create_buffer({
            .size = data.size(),
            .usage = usage,
            .location = gfx::buffer_location::device_local,
            .is_mutable = false,
        });

        m_newRenderer->buffer_upload_sync(buffer, data);

        return buffer;
    };

    // Vertex buffer
    {
        nstl::array<Vertex, 3> vertices = { {
            { {0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f} },
            { {-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} },
            { {0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} },
        } };

        testResources.vertexBuffer = createTestBuffer(gfx::buffer_usage::vertex_index, { vertices.data(), vertices.size() * sizeof(Vertex)});
    }

    // Index buffer
    {
        nstl::array<IndexT, 3> indices = { 0, 1, 2 };

        testResources.indexBuffer = createTestBuffer(gfx::buffer_usage::vertex_index, { indices.data(), indices.size() * sizeof(IndexT)});
    }

    // Frame uniform buffer and descriptor group
    {
        FrameUniformBuffer data = {

        };

        testResources.frameBuffer = createTestBuffer(gfx::buffer_usage::uniform, { &data, sizeof(data) });

        testResources.frameDescriptors = m_newRenderer->create_descriptorgroup({
            .entries = nstl::array{
                gfx::descriptorgroup_entry{ 0, testResources.frameBuffer },
            },
        });
    }

    // Material uniform buffer and descriptor group
    {
        MaterialUniformBuffer data = {

        };

        testResources.materialBuffer = createTestBuffer(gfx::buffer_usage::uniform, { &data, sizeof(data) });

        testResources.materialDescriptors = m_newRenderer->create_descriptorgroup({
            .entries = nstl::array{
                gfx::descriptorgroup_entry{ 0, testResources.materialBuffer },
            },
        });
    }

    // Object uniform buffer and descriptor group
    {
        ObjectUniformBuffer data = {

        };

        testResources.objectBuffer = createTestBuffer(gfx::buffer_usage::uniform, { &data, sizeof(data) });

        testResources.objectDescriptors = m_newRenderer->create_descriptorgroup({
            .entries = nstl::array{
                gfx::descriptorgroup_entry{ 0, testResources.objectBuffer },
            },
        });
    }

    // Shaders
    {
        {
            ShaderPackage shaders{ "data/shaders/packaged/test/test.vert" };
            testResources.vertexShader = m_newRenderer->create_shader({
                .filename = *shaders.get({}),
                .stage = gfx::shader_stage::vertex,
            });
        }
        {
            ShaderPackage shaders{ "data/shaders/packaged/test/test.frag" };
            testResources.fragmentShader = m_newRenderer->create_shader({
                .filename = *shaders.get({}),
                .stage = gfx::shader_stage::fragment,
            });
        }
    }

    // Render state
    {
        testResources.renderstate = m_newRenderer->create_renderstate({
            .shaders = nstl::array{ testResources.vertexShader, testResources.fragmentShader },
            .renderpass = m_newRenderer->get_main_renderpass(),
            .vertex_config = {
                .buffer_bindings = nstl::array{
                    gfx::buffer_binding_description{ .buffer_index = 0, .stride = sizeof(Vertex) },
                },
                .attributes = nstl::array{
                    gfx::attribute_description{ .location = 0, .buffer_binding_index = 0, .offset = offsetof(Vertex, position), .type = gfx::attribute_type::vec3f },
                    gfx::attribute_description{ .location = 1, .buffer_binding_index = 0, .offset = offsetof(Vertex, color), .type = gfx::attribute_type::vec3f },
                },
                .topology = gfx::vertex_topology::triangles,
            },
            .descriptorgroup_layouts = nstl::array{
                gfx::descriptorgroup_layout_view {
                    .entries = nstl::array{
                        gfx::descriptor_layout_entry{0, gfx::descriptor_type::buffer},
                    },
                },
                gfx::descriptorgroup_layout_view {
                    .entries = nstl::array{
                        gfx::descriptor_layout_entry{0, gfx::descriptor_type::buffer},
                    },
                },
                gfx::descriptorgroup_layout_view {
                    .entries = nstl::array{
                        gfx::descriptor_layout_entry{0, gfx::descriptor_type::buffer},
                    },
                },
            },
            .flags = {},
        });
    }
}

void DemoApplication::drawTest()
{
    m_newRenderer->wait_for_next_frame();
    m_newRenderer->begin_frame();

    m_newRenderer->renderpass_begin({
        .renderpass = m_newRenderer->get_main_renderpass(),
        .framebuffer = m_newRenderer->acquire_main_framebuffer(),
    });

    m_newRenderer->draw_indexed({
        .renderstate = testResources.renderstate,
        .descriptorgroups = nstl::array{ testResources.frameDescriptors, testResources.materialDescriptors, testResources.objectDescriptors },

        .vertex_buffers = nstl::array{ gfx::buffer_with_offset{ testResources.vertexBuffer } },
        .index_buffer = { testResources.indexBuffer },
        .index_type = gfx::index_type::uint16,

        .index_count = 3,
        .first_index = 0,
        .vertex_offset = 0,
    });

    m_newRenderer->renderpass_end();

    m_newRenderer->submit();
}
