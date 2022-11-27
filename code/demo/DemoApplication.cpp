#include "DemoApplication.h"

#include <memory>
#include <array>
#include <vector>
#include <sstream>
#include <iostream>
// #include <filesystem>
#include <thread>

#include "stb_image.h"
#include "glm.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "dds-ktx.h"
#include "cgltf.h"

#pragma warning(push, 0)
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#pragma warning(pop)

#include "wrapper/Sampler.h" // TODO remove
#include "wrapper/DebugMessage.h"
#include "wrapper/ShaderModule.h" // TODO remove, used only for enum
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

#include "services/DebugConsoleService.h"
#include "services/CommandLineService.h"
#include "services/DebugDrawService.h"

#include "common/Utils.h"

#include "nstl/array.h"
#include "nstl/span.h"
#include "nstl/optional.h"

// TODO find a better solution
template <>
struct magic_enum::customize::enum_range<vkr::GlfwWindow::Key> {
    static constexpr int min = 0;
    static constexpr int max = 10;
};
template <>
struct magic_enum::customize::enum_range<vkr::GlfwWindow::Modifiers> {
    static constexpr int min = 0;
    static constexpr int max = 10;
};
template <>
struct magic_enum::customize::enum_range<vkr::GlfwWindow::Action> {
    static constexpr int min = 0;
    static constexpr int max = 10;
};

// TODO move somewhere
template<>
struct fmt::formatter<nstl::string> : fmt::formatter<std::string_view>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(nstl::string const& str, FormatContext& ctx)
    {
        std::string_view sv{ str.data(), str.length() };
        return fmt::formatter<std::string_view>::format(sv, ctx);
    }
};
template<>
struct fmt::formatter<nstl::string_view> : fmt::formatter<std::string_view>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(nstl::string_view const& str, FormatContext& ctx)
    {
        std::string_view sv{ str.data(), str.length() };
        return fmt::formatter<std::string_view>::format(sv, ctx);
    }
};

namespace
{
    const uint32_t TARGET_WINDOW_WIDTH = 1900;
    const uint32_t TARGET_WINDOW_HEIGHT = 1000;

#ifdef _DEBUG
    bool const VALIDATION_ENABLED = true;
#else
    bool const VALIDATION_ENABLED = false;
#endif

    const glm::vec3 LIGHT_POS = glm::vec3(0.0, 2.0f, 0.0f);
    const glm::vec3 LIGHT_COLOR = glm::vec3(1.0, 1.0f, 1.0f);
    const float LIGHT_INTENSITY = 30.0f;
    const glm::vec3 CAMERA_POS = glm::vec3(0.0f, 0.0f, 4.0f);
    const glm::vec3 CAMERA_ANGLES = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::quat createRotation(glm::vec3 const& eulerDegrees)
    {
        return glm::quat{ glm::radians(eulerDegrees) };
    }

    glm::mat4 createMatrix(cgltf_node const& node)
    {
        if (node.has_matrix)
            return glm::make_mat4(node.matrix);

        auto matrix = glm::identity<glm::mat4>();

        if (node.has_translation)
        {
            glm::vec3 translation = glm::make_vec3(node.translation);
            matrix = glm::translate(matrix, translation);
        }

        if (node.has_rotation)
        {
            glm::quat rotation = glm::make_quat(node.rotation);
            matrix = matrix * glm::mat4_cast(rotation);
        }

        if (node.has_scale)
        {
            glm::vec3 scale = glm::make_vec3(node.scale);
            matrix = glm::scale(matrix, scale);
        }

        return matrix;
    }

    glm::vec4 createColor(nstl::span<float const> flatColor)
    {
        assert(flatColor.size() == 4);
        return glm::make_vec4(flatColor.data());
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

    nstl::optional<ImageData> loadWithStbImage(nstl::span<unsigned char const> bytes)
    {
        int w = 0, h = 0, comp = 0, req_comp = 4;
        unsigned char* data = stbi_load_from_memory(bytes.data(), bytes.size(), &w, &h, &comp, req_comp);
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
        std::copy(data, data + dataSize, imageData.bytes.begin());
        stbi_image_free(data);

        imageData.mips = { { 0, imageData.bytes.size() } };

        return imageData;
    }

    nstl::optional<ImageData> loadWithDdspp(nstl::span<unsigned char const> bytes)
    {
        ddsktx_texture_info info{};
        if (!ddsktx_parse(&info, bytes.data(), bytes.size(), nullptr))
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

        imageData.bytes.resize(bytes.size());
        memcpy(imageData.bytes.data(), bytes.data(), bytes.size());

        for (size_t mip = 0; mip < info.num_mips; mip++)
        {
            ddsktx_sub_data mipInfo;
            ddsktx_get_sub(&info, &mipInfo, bytes.data(), bytes.size(), 0, 0, mip);

            assert(mipInfo.buff > bytes.data());
            std::size_t offset = static_cast<unsigned char const*>(mipInfo.buff) - bytes.data();

            imageData.mips.push_back({ offset, static_cast<size_t>(mipInfo.size_bytes) });
        }

        return imageData;
    }

    nstl::optional<ImageData> loadImage(nstl::span<unsigned char const> bytes)
    {
        if (auto data = loadWithDdspp(bytes))
            return data;

        if (auto data = loadWithStbImage(bytes))
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

        throw std::invalid_argument("gltfAttributeType and gltfComponentType");
    }

    std::size_t getAttributeByteSize(vkgfx::AttributeType type)
    {
        std::size_t gltfFloatSize = 4;

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

        throw std::invalid_argument("type");
    }

    nstl::optional<std::size_t> findAttributeLocation(nstl::string_view name)
    {
        static nstl::vector<nstl::string_view> const attributeNames = { "POSITION", "COLOR_0", "TEXCOORD_0", "NORMAL", "TANGENT" }; // TODO move to the shader metadata

        auto it = attributeNames.find(name);

        if (it != attributeNames.end())
            return it - attributeNames.begin();

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
            *var = std::move(val);
            return *var;
        };

        return coil::overloaded(std::move(toggler), std::move(setter));
    }
}

//////////////////////////////////////////////////////////////////////////

DemoApplication::DemoApplication()
{
    m_services.setDebugConsole(std::make_unique<DebugConsoleService>(m_services));
    m_services.setCommandLine(std::make_unique<CommandLineService>(m_services));

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
        return glm::degrees(glm::eulerAngles(m_cameraTransform.rotation));
    }, [this](glm::vec3 const& angles) {
        m_cameraTransform.rotation = createRotation(angles);
    });

    m_commands["light.pos"] = coil::variable(&m_lightParameters.position);
    m_commands["light.color"] = coil::variable(&m_lightParameters.color);
    m_commands["light.intensity"] = coil::variable(&m_lightParameters.intensity);

    m_commands["fps"].description("Show/hide the FPS widget") = ::toggle(&m_showFps);
    m_commands["fps.update_period"].description("Update period of the FPS widget") = coil::variable(&m_fpsUpdatePeriod);

    m_keyState.resize(1 << 8 * sizeof(char), false);

    m_window = std::make_unique<vkr::GlfwWindow>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
    m_window->addKeyCallback([this](vkr::GlfwWindow::Action action, vkr::GlfwWindow::Key key, char c, vkr::GlfwWindow::Modifiers modifiers) { onKey(action, key, c, modifiers); });
    m_window->addMouseMoveCallback([this](glm::vec2 const& delta) { onMouseMove(delta); });

    auto messageCallback = [](vko::DebugMessage m)
    {
        // TODO don't log "Info" level to the console
// 		if (m.level == vko::DebugMessage::Level::Info)
// 			spdlog::info("{}", m.text);
		if (m.level == vko::DebugMessage::Level::Warning)
			spdlog::warn("{}", m.text);
		if (m.level == vko::DebugMessage::Level::Error)
			spdlog::error("{}", m.text);

        assert(m.level != vko::DebugMessage::Level::Error);
    };

    m_renderer = std::make_unique<vkgfx::Renderer>("Vulkan demo with new API", VALIDATION_ENABLED, *m_window, messageCallback);

    m_services.setDebugDraw(std::make_unique<DebugDrawService>(*m_renderer));

    loadImgui();

    m_commands["window.resize"].arguments("width", "height") = coil::bind(&vkr::GlfwWindow::resize, m_window.get());
    m_commands["window.width"] = coil::bindProperty(&vkr::GlfwWindow::getWidth, m_window.get());
    m_commands["window.height"] = coil::bindProperty(&vkr::GlfwWindow::getHeight, m_window.get());

    m_commands["scene.load"].description("Load scene from a GLTF model").arguments("path") = [this](coil::Context context, nstl::string_view path) {
        if (!loadScene(nstl::string{ path }))
            context.reportError("Failed to load the scene '" + coil::fromNstlStringView(path) + "'");
    };
    m_commands["scene.reload"] = [this]() { loadScene(m_currentScenePath); };
    m_commands["scene.unload"] = coil::bind(&DemoApplication::clearScene, this);

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

    createResources();
}

DemoApplication::~DemoApplication()
{
    clearScene();
    unloadImgui();

    // TODO come up with a better way to destroy objects with captured services
    m_debugConsole = {};
    m_notifications = {};
    m_commands.clear();

    m_services = Services{};
}

void DemoApplication::registerCommandLineOptions(CommandLineService& commandLine)
{
    commandLine.add("--execute")
        .default_value(std::vector<std::string>{})
        .append()
        .help("execute a given command");
}

bool DemoApplication::init(int argc, char** argv)
{
    auto& commandLine = m_services.commandLine();
    DemoApplication::registerCommandLineOptions(commandLine); // TODO move somewhere to allow others to register custom options

//     spdlog::info("Current directory: {}", std::filesystem::current_path());

//     if (!std::filesystem::exists("data"))
//         spdlog::warn("Current directory doesn't contain 'data', probably wrong directory");

    if (!commandLine.parse(argc, argv))
    {
        spdlog::critical("Failed to parse command line arguments");
        return false;
    }
    if (!commandLine.parseFile("data/cmdline.ini"))
    {
        spdlog::critical("Failed to parse cmdline.ini");
        return false;
    }

    {
        nstl::string args;
        for (nstl::string const& argument : commandLine.getAll())
            args += "'" + argument + "' ";

        spdlog::info("Command line arguments: {}", args);
    }

    m_notifications = ui::NotificationManager{ m_services };
    m_debugConsole = ui::DebugConsoleWidget{ m_services };

    return true;
}

void DemoApplication::run()
{
    auto lines = m_services.commandLine().get<std::vector<std::string>>("--execute");
    for (auto const& line : lines)
        m_services.debugConsole().execute(nstl::string_view{ line.data(), line.size() }); // TODO fix

    m_frameTimer.start();
    m_window->startEventLoop([this]() { drawFrame(); });
}

void DemoApplication::createResources()
{
    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    m_defaultVertexShader = std::make_unique<ShaderPackage>("data/shaders/packaged/shader.vert");
    m_defaultFragmentShader = std::make_unique<ShaderPackage>("data/shaders/packaged/shader.frag");

    m_defaultSampler = resourceManager.createSampler(vko::SamplerFilterMode::Linear, vko::SamplerFilterMode::Linear, vko::SamplerWrapMode::Repeat, vko::SamplerWrapMode::Repeat);

    m_defaultAlbedoImage = resourceManager.createImage(vkgfx::ImageMetadata{
        .width = 1,
        .height = 1,
        .byteSize = 4,
        .format = vkgfx::ImageFormat::R8G8B8A8,
    });
    resourceManager.uploadImage(m_defaultAlbedoImage, nstl::array<unsigned char, 4>{ 0xff, 0xff, 0xff, 0xff });
    m_defaultAlbedoTexture = resourceManager.createTexture(vkgfx::Texture{ m_defaultAlbedoImage, m_defaultSampler });

    m_defaultNormalMapImage = resourceManager.createImage(vkgfx::ImageMetadata{
        .width = 1,
        .height = 1,
        .byteSize = 4,
        .format = vkgfx::ImageFormat::R8G8B8A8,
    });
    resourceManager.uploadImage(m_defaultNormalMapImage, nstl::array<unsigned char, 4>{ 0x80, 0x80, 0xff, 0xff });
    m_defaultNormalMapTexture = resourceManager.createTexture(vkgfx::Texture{ m_defaultNormalMapImage, m_defaultSampler });
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

    m_imGuiDrawer = std::make_unique<ImGuiDrawer>(*m_renderer);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_window->getHandle(), true);
}

void DemoApplication::unloadImgui()
{
    if (!ImGui::GetCurrentContext())
        return;

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DemoApplication::onKey(vkr::GlfwWindow::Action action, vkr::GlfwWindow::Key key, char c, vkr::GlfwWindow::Modifiers mods)
{
    std::stringstream ss;
    auto separator = "";

    for (vkr::GlfwWindow::Modifiers value : magic_enum::enum_values<vkr::GlfwWindow::Modifiers>())
    {
        if (mods & value)
        {
            ss << separator << magic_enum::enum_name(value);
            separator = " | ";
        }
    }

    std::cout << magic_enum::enum_name(action) << ' ' << magic_enum::enum_name(key) << ' ' << ss.str() << ": " << "'" << c << "'" << std::endl;

    std::size_t index = static_cast<std::size_t>(c);
    m_keyState[index] = action == vkr::GlfwWindow::Action::Press;
    m_modifiers = mods;

    if (c == '`' && action == vkr::GlfwWindow::Action::Press)
        m_debugConsole->toggle();
}

void DemoApplication::onMouseMove(glm::vec2 const& delta)
{
    glm::vec3 angleDelta = m_mouseSensitivity * glm::vec3{ -delta.y, -delta.x, 0.0f };
    glm::quat rotationDelta = createRotation(angleDelta);

    m_cameraTransform.rotation *= rotationDelta;
}

DemoScene DemoApplication::createDemoScene(cgltf_data const& gltfModel, cgltf_scene const& gltfScene) const
{
    DemoScene scene;

    for (size_t i = 0; i < gltfScene.nodes_count; i++)
        createDemoObjectRecursive(gltfModel, i, glm::identity<glm::mat4>(), scene);

    return scene;
}

void DemoApplication::createDemoObjectRecursive(cgltf_data const& gltfModel, std::size_t nodeIndex, glm::mat4 parentTransform, DemoScene& scene) const
{
    // TODO is there a better way?
    auto findIndex = [](auto const* object, auto const* firstObject, size_t count) -> size_t
    {
        auto index = object - firstObject;
        assert(index >= 0);
        assert(index < count);
        return static_cast<size_t>(index);
    };

    struct DemoObjectPushConstants
    {
        glm::mat4 model;
    };

    struct DemoObjectUniformBuffer
    {
        glm::vec4 color;
    };

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    cgltf_node const& gltfNode = gltfModel.nodes[nodeIndex];

    glm::mat4 nodeTransform = parentTransform * createMatrix(gltfNode);

    if (gltfNode.mesh)
    {
        size_t meshIndex = findIndex(gltfNode.mesh, gltfModel.meshes, gltfModel.meshes_count);

        for (DemoMesh const& mesh : m_gltfResources->meshes[meshIndex])
        {
            DemoMaterial const& material = m_gltfResources->materials[mesh.metadata.materialIndex];

            vkgfx::PipelineKey pipelineKey;
            pipelineKey.renderConfig = material.metadata.renderConfig;

            // TODO think how to handle multiple descriptor set layouts properly
            pipelineKey.uniformConfigs = {
                // TODO get shared frame uniform config from the Renderer
                vkgfx::UniformConfiguration{
                    .hasBuffer = true,
                    .hasAlbedoTexture = false,
                    .hasNormalMap = false,
                },
                material.metadata.uniformConfig,
                vkgfx::UniformConfiguration{
                    .hasBuffer = true,
                    .hasAlbedoTexture = false,
                    .hasNormalMap = false,
                },
            };

            pipelineKey.vertexConfig = mesh.metadata.vertexConfig;

            pipelineKey.pushConstantRanges = { vkgfx::PushConstantRange{.offset = 0, .size = sizeof(DemoObjectPushConstants), } };

            // TODO reimplement
            ShaderConfiguration shaderConfiguration;
            shaderConfiguration.hasTexture = material.metadata.uniformConfig.hasAlbedoTexture;
            shaderConfiguration.hasNormalMap = material.metadata.uniformConfig.hasNormalMap;
            shaderConfiguration.hasColor = mesh.metadata.attributeSemanticsConfig.hasColor;
            shaderConfiguration.hasTexCoord = mesh.metadata.attributeSemanticsConfig.hasUv;
            shaderConfiguration.hasNormal = mesh.metadata.attributeSemanticsConfig.hasNormal;
            shaderConfiguration.hasTangent = mesh.metadata.attributeSemanticsConfig.hasTangent;

            nstl::string const* vertexShaderPath = m_defaultVertexShader->get(shaderConfiguration);
            nstl::string const* fragmentShaderPath = m_defaultFragmentShader->get(shaderConfiguration);

            if (!vertexShaderPath || !fragmentShaderPath)
                throw std::runtime_error("Failed to find the shader");

            vkgfx::ShaderModuleHandle vertexShaderModule = m_gltfResources->shaderModules[*vertexShaderPath];
            vkgfx::ShaderModuleHandle fragmentShaderModule = m_gltfResources->shaderModules[*fragmentShaderPath];

            pipelineKey.shaderHandles = { vertexShaderModule, fragmentShaderModule };

            vkgfx::PipelineHandle pipeline = resourceManager.getOrCreatePipeline(pipelineKey);

            vkgfx::BufferMetadata uniformBufferMetadata{
                .usage = vkgfx::BufferUsage::UniformBuffer,
                .location = vkgfx::BufferLocation::HostVisible,
                .isMutable = false,
            };
            vkgfx::BufferHandle uniformBuffer = resourceManager.createBuffer(sizeof(DemoObjectUniformBuffer), std::move(uniformBufferMetadata));
            m_gltfResources->additionalBuffers.push_back(uniformBuffer);

            DemoObjectUniformBuffer uniformValues;
            uniformValues.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            resourceManager.uploadBuffer(uniformBuffer, &uniformValues, sizeof(uniformValues));

            DemoObjectPushConstants pushConstants;
            pushConstants.model = nodeTransform;

            vkgfx::TestObject& object = scene.objects.emplace_back();
            object.mesh = mesh.handle;
            object.material = material.handle;
            object.pipeline = pipeline;
            object.uniformBuffer = uniformBuffer;
            object.pushConstants.resize(sizeof(pushConstants));
            memcpy(object.pushConstants.data(), &pushConstants, object.pushConstants.size());
        }
    }

    if (gltfNode.camera)
    {
        glm::vec3 position;
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(nodeTransform, scale, rotation, position, skew, perspective);

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
        for (auto const& meshList : resources.meshes)
            for (auto const& mesh : meshList)
                resourceManager.removeMesh(mesh.handle);
        for (auto const& handle : resources.samplers)
            resourceManager.removeSampler(handle);
        for (auto const& [name, handle] : resources.shaderModules)
            resourceManager.removeShaderModule(handle);
        for (auto const& handle : resources.textures)
            resourceManager.removeTexture(handle);

        m_gltfResources = {};
    }
}

bool DemoApplication::loadScene(nstl::string const& gltfPath)
{
    clearScene();

    if (gltfPath.empty())
        return false;

    m_currentScenePath = gltfPath;

    auto buffer = vkc::utils::readFile(gltfPath.c_str());

    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse(&options, buffer.data(), buffer.size(), &data);
    if (result != cgltf_result_success)
        return false;

    nstl::string basePath = "";
    if (auto pos = gltfPath.find_last_of("/\\"); pos != std::string::npos)
        basePath = gltfPath.substr(0, pos + 1);

    cgltf_load_buffers(&options, data, basePath.c_str());

    assert(data);
    loadGltfModel(basePath, *data);

    cgltf_free(data);

    return true;
}

bool DemoApplication::loadGltfModel(nstl::string_view basePath, cgltf_data const& model)
{
    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    m_gltfResources = std::make_unique<GltfResources>();

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
        auto index = object - firstObject;
        assert(index >= 0);
        assert(index < count);
        return static_cast<size_t>(index);
    };

    for (auto i = 0; i < model.extensions_required_count; i++)
        spdlog::warn("GLTF requires extension '{}'", model.extensions_required[i]);

    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultVertexShader->getAll())
    for (auto const& pair : m_defaultVertexShader->getAll())
    {
        auto const& configuration = pair.key();
        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readFile(modulePath.c_str()), vko::ShaderModuleType::Vertex, "main");
        m_gltfResources->shaderModules.insert_or_assign(modulePath, handle);
    }
    // TODO implement
//     for (auto const& [configuration, modulePath] : m_defaultFragmentShader->getAll())
    for (auto const& pair : m_defaultFragmentShader->getAll())
    {
        auto const& configuration = pair.key();
        auto const& modulePath = pair.value();
        auto handle = resourceManager.createShaderModule(vkc::utils::readFile(modulePath.c_str()), vko::ShaderModuleType::Fragment, "main");
        m_gltfResources->shaderModules.insert_or_assign(modulePath, handle);
    }

    m_gltfResources->buffers.reserve(model.buffers_count);
    for (auto i = 0; i < model.buffers_count; i++)
    {
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
        auto handle = resourceManager.createBuffer(bufferSize, std::move(metadata)); // TODO split buffer into several different parts
        resourceManager.uploadBuffer(handle, data);
        m_gltfResources->buffers.push_back(handle);
    }

    for (auto i = 0; i < model.samplers_count; i++)
    {
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
    for (auto i = 0; i < model.images_count; i++)
    {
        cgltf_image const& gltfImage = model.images[i];
        assert(gltfImage.uri && !gltfImage.buffer_view);

        nstl::string imagePath = basePath + gltfImage.uri;

        // TODO leads to unnecessary data copy; change that
        nstl::optional<ImageData> imageData = loadImage(vkc::utils::readFile(imagePath.c_str()));
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

    for (auto i = 0; i < model.textures_count; i++)
    {
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

        vkgfx::TextureHandle handle = resourceManager.createTexture(std::move(texture));
        m_gltfResources->textures.push_back(handle);
    }

    for (auto i = 0; i < model.materials_count; i++)
    {
        cgltf_material const& gltfMaterial = model.materials[i];

        cgltf_pbr_metallic_roughness const& gltfRoughness = gltfMaterial.pbr_metallic_roughness;

        vkgfx::Material material;
        material.albedo = m_defaultAlbedoTexture;
        material.normalMap = m_defaultNormalMapTexture;

        struct MaterialUniformBuffer
        {
            glm::vec4 color;
        };

        MaterialUniformBuffer values;
        values.color = createColor(gltfRoughness.base_color_factor);

        vkgfx::BufferMetadata metadata{
            .usage = vkgfx::BufferUsage::UniformBuffer,
            .location = vkgfx::BufferLocation::HostVisible,
            .isMutable = false,
        };
        auto buffer = resourceManager.createBuffer(sizeof(MaterialUniformBuffer), std::move(metadata));
        resourceManager.uploadBuffer(buffer, &values, sizeof(MaterialUniformBuffer));
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

        demoMaterial.handle = resourceManager.createMaterial(std::move(material));

        demoMaterial.metadata.renderConfig.wireframe = false;
        demoMaterial.metadata.renderConfig.cullBackfaces = !gltfMaterial.double_sided;
        demoMaterial.metadata.uniformConfig.hasBuffer = true;
        demoMaterial.metadata.uniformConfig.hasAlbedoTexture = true;
        demoMaterial.metadata.uniformConfig.hasNormalMap = true;
    }

    m_gltfResources->meshes.reserve(model.meshes_count);
    for (auto meshIndex = 0; meshIndex < model.meshes_count; meshIndex++)
    {
        cgltf_mesh const& gltfMesh = model.meshes[meshIndex];

        nstl::vector<DemoMesh>& demoMeshes = m_gltfResources->meshes.emplace_back();
        demoMeshes.reserve(gltfMesh.primitives_count);
        for (auto primitiveIndex = 0; primitiveIndex < gltfMesh.primitives_count; primitiveIndex++)
        {
            cgltf_primitive const& gltfPrimitive = gltfMesh.primitives[primitiveIndex];

            DemoMesh& demoMesh = demoMeshes.emplace_back();

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
            demoMesh.metadata.vertexConfig.bindings.reserve(gltfPrimitive.attributes_count);
            demoMesh.metadata.vertexConfig.attributes.reserve(gltfPrimitive.attributes_count);

            for (size_t attributeIndex = 0; attributeIndex < gltfPrimitive.attributes_count; attributeIndex++)
            {
                cgltf_attribute const& gltfAttribute = gltfPrimitive.attributes[attributeIndex];

                nstl::string_view name = gltfAttribute.name;

                nstl::optional<std::size_t> location = findAttributeLocation(name);

                if (!location)
                {
                    spdlog::warn("Skipping attribute '{}'", name);
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
                    demoMesh.metadata.attributeSemanticsConfig.hasColor = true;
                if (name == "TEXCOORD_0")
                    demoMesh.metadata.attributeSemanticsConfig.hasUv = true;
                if (name == "NORMAL")
                    demoMesh.metadata.attributeSemanticsConfig.hasNormal = true;
                if (name == "TANGENT")
                    demoMesh.metadata.attributeSemanticsConfig.hasTangent = true;

                vkgfx::AttributeType attributeType = findAttributeType(gltfAccessor->type, gltfAccessor->component_type);

                std::size_t stride = gltfBufferView->stride;
                if (stride == 0)
                    stride = getAttributeByteSize(attributeType);

                vkgfx::VertexConfiguration::Binding& bindingConfig = demoMesh.metadata.vertexConfig.bindings.emplace_back();
                bindingConfig.stride = stride;

                vkgfx::VertexConfiguration::Attribute& attributeConfig = demoMesh.metadata.vertexConfig.attributes.emplace_back();
                attributeConfig.binding = attributeIndex;
                attributeConfig.location = *location;
                attributeConfig.offset = 0; // TODO can be improved
                attributeConfig.type = attributeType;

                // TODO implement
                assert(gltfPrimitive.type == cgltf_primitive_type_triangles);
                demoMesh.metadata.vertexConfig.topology = vkgfx::VertexTopology::Triangles;

                demoMesh.metadata.materialIndex = findIndex(gltfPrimitive.material, model.materials, model.materials_count); // TODO check
            }

            demoMesh.handle = resourceManager.createMesh(std::move(mesh));
        }
    }

    for (size_t i = 0; i < model.cameras_count; i++)
    {
        cgltf_camera const& gltfCamera = model.cameras[i];

        if (gltfCamera.type == cgltf_camera_type_perspective)
        {
            cgltf_camera_perspective const& gltfParams = gltfCamera.data.perspective;

            assert(gltfParams.has_zfar);

            m_gltfResources->cameraParameters.push_back(vkgfx::TestCameraParameters{
                .fov = glm::degrees(static_cast<float>(gltfParams.yfov)),
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

        std::sort(m_demoScene.objects.begin(), m_demoScene.objects.end(), [](vkgfx::TestObject const& lhs, vkgfx::TestObject const& rhs)
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

    ImGui_ImplGlfw_NewFrame();
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
        static std::size_t nextIndex = 0;
        if (ImGui::Button("Add notification"))
        {
            m_notifications->add(texts[nextIndex % texts.size()]);
            nextIndex++;
        }
        ImGui::End();
    }

    m_notifications->draw();
    m_debugConsole->draw();

    // TODO fix this logic
    m_window->setCanCaptureCursor(!io.WantCaptureMouse);
}

void DemoApplication::drawFrame()
{
    update();

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
    m_services.debugDraw().box(m_lightParameters.position, glm::identity<glm::quat>(), glm::vec3(0.1f), { 1.0f, 0.0f, 0.0f }, -1.0f);
}

void DemoApplication::updateCamera(float dt)
{
    // TODO handle input properly
    if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
        return;

    glm::vec3 right = glm::toMat4(m_cameraTransform.rotation) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 forward = glm::toMat4(m_cameraTransform.rotation) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    glm::vec3 up = glm::toMat4(m_cameraTransform.rotation) * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    glm::vec3 posDelta = glm::zero<glm::vec3>();

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

    if (glm::length2(posDelta) > glm::epsilon<float>())
        posDelta = glm::normalize(posDelta);

    m_cameraTransform.position += m_cameraSpeed * dt * posDelta;
}
