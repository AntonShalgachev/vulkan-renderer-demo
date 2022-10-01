#include "DemoApplication.h"

#include <memory>
#include <array>
#include <vector>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>

#include "stb_image.h"
#include "glm.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

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

    glm::mat4 createMatrix(tinygltf::Node const& node)
    {
        if (!node.matrix.empty() && node.matrix.size() != 16)
            throw std::runtime_error("unexpected linear matrix size");

        if (!node.scale.empty() && node.scale.size() != 3)
            throw std::runtime_error("unexpected linear scale size");

        if (!node.translation.empty() && node.translation.size() != 3)
            throw std::runtime_error("unexpected linear translation size");

        if (!node.rotation.empty() && node.rotation.size() != 4)
            throw std::runtime_error("unexpected linear rotation size");

        if (!node.matrix.empty())
            return glm::make_mat4(node.matrix.data());

        auto matrix = glm::identity<glm::mat4>();

        if (!node.translation.empty())
        {
            glm::vec3 translation = glm::make_vec3(node.translation.data());
            matrix = glm::translate(matrix, translation);
        }

        if (!node.rotation.empty())
        {
            glm::quat rotation = glm::make_quat(node.rotation.data());
            matrix = matrix * glm::mat4_cast(rotation);
        }

        if (!node.scale.empty())
        {
            glm::vec3 scale = glm::make_vec3(node.scale.data());
            matrix = glm::scale(matrix, scale);
        }

        return matrix;
    }

    glm::vec4 createColor(std::vector<double> const& flatColor)
    {
        if (flatColor.empty())
            return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (flatColor.size() == 4)
            return glm::make_vec4(flatColor.data());

        throw std::runtime_error("unexpected flat color size");
    }

    bool loadImage(tinygltf::Image& image)
    {
        auto const* bytes = image.image.data();
        auto size = image.image.size();

        int w = 0, h = 0, comp = 0, req_comp = 4;
        unsigned char* data = stbi_load_from_memory(bytes, size, &w, &h, &comp, req_comp);
        int bits = 8;
        int pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

        if (!data)
            return false;

        if (w < 1 || h < 1)
        {
            stbi_image_free(data);
            return false;
        }

        if (req_comp != 0)
        {
            // loaded data has `req_comp` channels(components)
            comp = req_comp;
        }

        image.width = w;
        image.height = h;
        image.component = comp;
        image.bits = bits;
        image.pixel_type = pixel_type;
        image.as_is = false;
        image.image.resize(static_cast<size_t>(w * h * comp) * size_t(bits / 8));
        std::copy(data, data + w * h * comp * (bits / 8), image.image.begin());
        stbi_image_free(data);

        return true;
    }

    std::optional<tinygltf::Model> loadModel(std::string const& path)
    {
        tinygltf::Model model;

        tinygltf::TinyGLTF loader;
        loader.SetImageLoader([](tinygltf::Image* image, const int image_idx, std::string* err,
                                 std::string*, int req_width, int req_height,
                                 const unsigned char* bytes, int size, void*)
        {
            if (size < 1)
                return false;

            image->image.resize(size_t(size));
            memcpy(image->image.data(), bytes, size_t(size));
            image->as_is = true;

            return true;
        }, nullptr);

        std::string err;
        std::string warn;
        bool success = loader.LoadASCIIFromFile(&model, &err, &warn, path);

        if (!warn.empty())
            std::cout << warn << std::endl;
        if (!err.empty())
            std::cout << err << std::endl;

        if (!success)
            return {};

        return model;
    }

    vkgfx::IndexType findIndexType(int gltfComponentType)
    {
        switch (gltfComponentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return vkgfx::IndexType::UnsignedByte;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return vkgfx::IndexType::UnsignedShort;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return vkgfx::IndexType::UnsignedInt;
        }

        assert(false);
        return vkgfx::IndexType::UnsignedShort;
    }

    // TODO rename
    vkgfx::AttributeType findAttributeType2(int gltfAttributeType, int gltfComponentType)
    {
        if (gltfAttributeType == TINYGLTF_TYPE_VEC2 && gltfComponentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
            return vkgfx::AttributeType::Vec2f;
        if (gltfAttributeType == TINYGLTF_TYPE_VEC3 && gltfComponentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
            return vkgfx::AttributeType::Vec3f;
        if (gltfAttributeType == TINYGLTF_TYPE_VEC4 && gltfComponentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
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
        case vkgfx::AttributeType::Mat2f:
            return 4 * gltfFloatSize;
        case vkgfx::AttributeType::Mat3f:
            return 9 * gltfFloatSize;
        case vkgfx::AttributeType::Mat4f:
            return 16 * gltfFloatSize;
        }

        throw std::invalid_argument("type");
    }

    std::size_t findAttributeLocation(std::string const& name)
    {
        static std::vector<std::string> const attributeNames = { "POSITION", "COLOR_0", "TEXCOORD_0", "NORMAL", "TANGENT" }; // TODO move to the shader metadata

        auto it = std::find(attributeNames.cbegin(), attributeNames.cend(), name);

        if (it != attributeNames.end())
            return static_cast<std::size_t>(std::distance(attributeNames.begin(), it));

        throw std::runtime_error("Unkown attribute name: " + name);
    }

    std::vector<unsigned char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        std::streamsize fileSize = file.tellg();
        std::vector<unsigned char> buffer(static_cast<std::size_t>(fileSize));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize); // safe, since char and unsigned char have the same alignment and representation
        file.close();

        return buffer;
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
    createServices();

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
    m_window->addResizeCallback([this](int, int) { onFramebufferResized(); });
    m_window->addKeyCallback([this](vkr::GlfwWindow::Action action, vkr::GlfwWindow::Key key, char c, vkr::GlfwWindow::Modifiers modifiers) { onKey(action, key, c, modifiers); });
    m_window->addMouseMoveCallback([this](glm::vec2 const& delta) { onMouseMove(delta); });

    auto messageCallback = [](vko::DebugMessage m)
    {
        // TODO don't log "Info" level to the console
// 		if (m.level == vko::DebugMessage::Level::Info)
// 			spdlog::info(m.text);
		if (m.level == vko::DebugMessage::Level::Warning)
			spdlog::warn(m.text);
		if (m.level == vko::DebugMessage::Level::Error)
			spdlog::error(m.text);

        assert(m.level != vko::DebugMessage::Level::Error);
    };

    m_renderer = std::make_unique<vkgfx::Renderer>("Vulkan demo with new API", VALIDATION_ENABLED, *m_window, messageCallback);

    m_services.debugDraw().init(*m_renderer);

    loadImgui();

    m_commands["window.resize"].arguments("width", "height") = coil::bind(&vkr::GlfwWindow::resize, m_window.get());
    m_commands["window.width"] = coil::bindProperty(&vkr::GlfwWindow::getWidth, m_window.get());
    m_commands["window.height"] = coil::bindProperty(&vkr::GlfwWindow::getHeight, m_window.get());

    m_commands["scene.load"].description("Load scene from a GLTF model").arguments("path") = [this](coil::Context context, std::string_view path) {
        std::string pathStr{ path };
        if (!loadScene(pathStr))
            context.reportError("Failed to load the scene '" + pathStr + "'");
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

    destroyServices();
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

    spdlog::info("Current directory: {}", std::filesystem::current_path());

    if (!std::filesystem::exists("data"))
        spdlog::warn("Current directory doesn't contain 'data', probably wrong directory");

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
        std::stringstream ss;
        for (std::string const& argument : commandLine.getAll())
            ss << "'" << argument << "' ";

        spdlog::info("Command line arguments: {}", ss.str());
    }

    m_notifications = ui::NotificationManager{ m_services };
    m_debugConsole = ui::DebugConsoleWidget{ m_services };

    return true;
}

void DemoApplication::run()
{
    auto lines = m_services.commandLine().get<std::vector<std::string>>("--execute");
    for (auto const& line : lines)
        m_services.debugConsole().execute(line);

    m_frameTimer.start();
    m_window->startEventLoop([this]() { drawFrame(); });
}

void DemoApplication::createServices()
{
    // TODO don't pass vkr::Application like this. Either remove it completely or add to services
    m_services.setDebugConsole(std::make_unique<DebugConsoleService>(m_services));
    m_services.setCommandLine(std::make_unique<CommandLineService>(m_services));
    m_services.setDebugDraw(std::make_unique<DebugDrawService>(m_services));
}

void DemoApplication::destroyServices()
{
    m_services.setCommandLine(nullptr);
    m_services.setDebugConsole(nullptr);
    m_services.setDebugDraw(nullptr);
}

void DemoApplication::createResources()
{
    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    m_defaultVertexShader = std::make_unique<ShaderPackage>("data/shaders/packaged/shader.vert");
    m_defaultFragmentShader = std::make_unique<ShaderPackage>("data/shaders/packaged/shader.frag");

    m_defaultSampler = resourceManager.createSampler(vko::SamplerFilterMode::Linear, vko::SamplerFilterMode::Linear, vko::SamplerWrapMode::Repeat, vko::SamplerWrapMode::Repeat);

    m_defaultAlbedoImage = resourceManager.createImage(vkgfx::ImageMetadata{ 1, 1, vkgfx::ImageFormat::R8G8B8A8 });
    resourceManager.uploadImage(m_defaultAlbedoImage, std::array<unsigned char, 4>{ 0xff, 0xff, 0xff, 0xff });
    m_defaultAlbedoTexture = resourceManager.createTexture(vkgfx::Texture{ m_defaultAlbedoImage, m_defaultSampler });

    m_defaultNormalMapImage = resourceManager.createImage(vkgfx::ImageMetadata{ 1, 1, vkgfx::ImageFormat::R8G8B8A8 });
    resourceManager.uploadImage(m_defaultNormalMapImage, std::array<unsigned char, 4>{ 0x80, 0x80, 0xff, 0xff });
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

void DemoApplication::onFramebufferResized()
{
    // TODO remove
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

DemoScene DemoApplication::createDemoScene(tinygltf::Model const& gltfModel, tinygltf::Scene const& gltfScene) const
{
    DemoScene scene;

    for (std::size_t nodeIndex : gltfScene.nodes)
        createDemoObjectRecursive(gltfModel, nodeIndex, scene);

    return scene;
}

void DemoApplication::createDemoObjectRecursive(tinygltf::Model const& gltfModel, std::size_t nodeIndex, DemoScene& scene) const
{
    struct DemoObjectPushConstants
    {
        glm::mat4 model;
    };

    struct DemoObjectUniformBuffer
    {
        glm::vec4 color;
    };

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    tinygltf::Node const& gltfNode = gltfModel.nodes[nodeIndex];

    if (gltfNode.mesh >= 0)
    {
        std::size_t meshIndex = static_cast<std::size_t>(gltfNode.mesh);

        for (DemoMesh const& mesh : m_gltfResources->meshes[meshIndex])
        {
            DemoMaterial const& material = m_gltfResources->materials[mesh.metadata.materialIndex];

            vkgfx::PipelineKey pipelineKey;
            pipelineKey.renderConfig = material.metadata.renderConfig;

            // TODO think how to handle multiple descriptor set layouts properly
            pipelineKey.uniformConfigs = {
                // WTF get shared frame uniform config from the Renderer
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

            std::string const* vertexShaderPath = m_defaultVertexShader->get(shaderConfiguration);
            std::string const* fragmentShaderPath = m_defaultFragmentShader->get(shaderConfiguration);

            if (!vertexShaderPath || !fragmentShaderPath)
                throw std::runtime_error("Failed to find the shader");

            vkgfx::ShaderModuleHandle vertexShaderModule = m_gltfResources->shaderModules[*vertexShaderPath];
            vkgfx::ShaderModuleHandle fragmentShaderModule = m_gltfResources->shaderModules[*fragmentShaderPath];

            pipelineKey.shaderHandles = { vertexShaderModule, fragmentShaderModule };

            vkgfx::PipelineHandle pipeline = resourceManager.getOrCreatePipeline(pipelineKey);

            vkgfx::BufferMetadata uniformBufferMetadata{
                .usage = vkgfx::BufferUsage::UniformBuffer,
                .location = vkgfx::BufferLocation::HostVisible,
                .isMutable = false, // TODO change when mutable buffers are implemented
            };
            vkgfx::BufferHandle uniformBuffer = resourceManager.createBuffer(sizeof(DemoObjectUniformBuffer), std::move(uniformBufferMetadata));
            m_gltfResources->additionalBuffers.push_back(uniformBuffer);

            DemoObjectUniformBuffer uniformValues;
            uniformValues.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            resourceManager.uploadBuffer(uniformBuffer, &uniformValues, sizeof(uniformValues));

            DemoObjectPushConstants pushConstants;
            pushConstants.model = createMatrix(gltfNode); // TODO take hierarchy into account

            vkgfx::TestObject& object = scene.objects.emplace_back();
            object.mesh = mesh.handle;
            object.material = material.handle;
            object.pipeline = pipeline;
            object.uniformBuffer = uniformBuffer;
            object.pushConstants.resize(sizeof(pushConstants));
            memcpy(object.pushConstants.data(), &pushConstants, object.pushConstants.size());
        }
    }

    for (auto childNodeIndex : gltfNode.children)
    {
        createDemoObjectRecursive(gltfModel, static_cast<std::size_t>(childNodeIndex), scene);
    }
}

void DemoApplication::clearScene()
{
    for (std::thread& thread : m_imageLoadingThreads)
        thread.join();
    m_imageLoadingThreads.clear();

    m_renderer->clearObjects();
    m_renderer->waitIdle(); // TODO remove

    if (m_gltfResources)
    {
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
    }
}

bool DemoApplication::loadScene(std::string const& gltfPath)
{
    clearScene();

    if (gltfPath.empty())
        return false;

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    m_gltfModel = loadModel(gltfPath);

    if (!m_gltfModel)
        return false;

    m_gltfResources = std::make_unique<GltfResources>();

    for (auto const& [configuration, modulePath] : m_defaultVertexShader->getAll())
    {
        auto handle = resourceManager.createShaderModule(readFile(modulePath), vko::ShaderModuleType::Vertex, "main");
        m_gltfResources->shaderModules[modulePath] = handle;
    }
    for (auto const& [configuration, modulePath] : m_defaultFragmentShader->getAll())
    {
        auto handle = resourceManager.createShaderModule(readFile(modulePath), vko::ShaderModuleType::Fragment, "main");
        m_gltfResources->shaderModules[modulePath] = handle;
    }

    for (tinygltf::Buffer const& buffer : m_gltfModel->buffers)
    {
        auto const& data = buffer.data;
        VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

        vkgfx::BufferMetadata metadata{
            .usage = vkgfx::BufferUsage::VertexIndexBuffer,
            .location = vkgfx::BufferLocation::DeviceLocal,
            .isMutable = false,
        };
        auto handle = resourceManager.createBuffer(bufferSize, std::move(metadata)); // TODO split buffer into several different parts
        resourceManager.uploadBuffer(handle, data);
        m_gltfResources->buffers.push_back(handle);
    }

    for (tinygltf::Sampler const& gltfSampler : m_gltfModel->samplers)
    {
        auto convertFilterMode = [](int gltfMode)
        {
            switch (gltfMode)
            {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
                return vko::SamplerFilterMode::Nearest;
            case -1:
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
                return vko::SamplerFilterMode::Linear;
            }

            throw std::invalid_argument("gltfMode");
        };

        auto convertWrapMode = [](int gltfMode)
        {
            switch (gltfMode)
            {
            case -1:
            case TINYGLTF_TEXTURE_WRAP_REPEAT:
                return vko::SamplerWrapMode::Repeat;
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                return vko::SamplerWrapMode::ClampToEdge;
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                return vko::SamplerWrapMode::Mirror;
            }

            throw std::invalid_argument("gltfMode");
        };

        auto magFilter = convertFilterMode(gltfSampler.magFilter);
        auto minFilter = convertFilterMode(gltfSampler.minFilter);
        auto wrapU = convertWrapMode(gltfSampler.wrapS);
        auto wrapV = convertWrapMode(gltfSampler.wrapT);

        auto handle = resourceManager.createSampler(magFilter, minFilter, wrapU, wrapV);
        m_gltfResources->samplers.push_back(handle);
    }

    if (m_imageReadyFlags.size() < m_gltfModel->images.size())
        m_imageReadyFlags.resize(m_gltfModel->images.size());
    std::fill(m_imageReadyFlags.begin(), m_imageReadyFlags.end(), false);
    m_imageReadyFlagsChanged = false;

    for (std::size_t i = 0; i < m_gltfModel->images.size(); i++)
    {
        m_imageLoadingThreads.emplace_back([this, i]()
        {
            assert(m_gltfModel);
            loadImage(m_gltfModel->images[i]);
            assert(m_gltfModel);
            std::lock_guard<std::mutex> guard{ m_imageReadyFlagsMutex };
            m_imageReadyFlags[i] = true;
            m_imageReadyFlagsChanged = true;
        });
    }

    m_gltfResources->images.resize(m_gltfModel->images.size());
    m_gltfResources->textures.resize(m_gltfModel->textures.size());

    for (tinygltf::Material const& gltfMaterial : m_gltfModel->materials)
    {
        tinygltf::PbrMetallicRoughness const& gltfRoughness = gltfMaterial.pbrMetallicRoughness;

        vkgfx::Material material;
        material.albedo = m_defaultAlbedoTexture;
        material.normalMap = m_defaultNormalMapTexture;

        struct MaterialUniformBuffer
        {
            glm::vec4 color;
        };

        MaterialUniformBuffer values;
        values.color = createColor(gltfRoughness.baseColorFactor);

        vkgfx::BufferMetadata metadata{
            .usage = vkgfx::BufferUsage::UniformBuffer,
            .location = vkgfx::BufferLocation::HostVisible,
            .isMutable = false,
        };
        auto buffer = resourceManager.createBuffer(sizeof(MaterialUniformBuffer), std::move(metadata));
        resourceManager.uploadBuffer(buffer, &values, sizeof(MaterialUniformBuffer));
        m_gltfResources->additionalBuffers.push_back(buffer);

        material.uniformBuffer = buffer;

        DemoMaterial& demoMaterial = m_gltfResources->materials.emplace_back();

        demoMaterial.handle = resourceManager.createMaterial(std::move(material));

        demoMaterial.metadata.renderConfig.wireframe = false;
        demoMaterial.metadata.renderConfig.cullBackfaces = !gltfMaterial.doubleSided;
        demoMaterial.metadata.uniformConfig.hasBuffer = true;
        demoMaterial.metadata.uniformConfig.hasAlbedoTexture = true;
        demoMaterial.metadata.uniformConfig.hasNormalMap = true;
    }

    for (tinygltf::Mesh const& gltfMesh : m_gltfModel->meshes)
    {
        std::vector<DemoMesh>& demoMeshes = m_gltfResources->meshes.emplace_back();
        demoMeshes.reserve(gltfMesh.primitives.size());

        for (tinygltf::Primitive const& gltfPrimitive : gltfMesh.primitives)
        {
            DemoMesh& demoMesh = demoMeshes.emplace_back();

            vkgfx::Mesh mesh;

            {
                tinygltf::Accessor const& gltfAccessor = m_gltfModel->accessors[static_cast<std::size_t>(gltfPrimitive.indices)];
                tinygltf::BufferView const& gltfBufferView = m_gltfModel->bufferViews[static_cast<std::size_t>(gltfAccessor.bufferView)];

                mesh.indexBuffer.buffer = m_gltfResources->buffers[static_cast<std::size_t>(gltfBufferView.buffer)];
                mesh.indexBuffer.offset = gltfBufferView.byteOffset + gltfAccessor.byteOffset;
                mesh.indexCount = gltfAccessor.count;
                mesh.indexType = findIndexType(gltfAccessor.componentType);
            }

            mesh.vertexBuffers.reserve(gltfPrimitive.attributes.size());
            demoMesh.metadata.vertexConfig.bindings.reserve(gltfPrimitive.attributes.size());
            demoMesh.metadata.vertexConfig.attributes.reserve(gltfPrimitive.attributes.size());

            std::size_t attributeIndex = 0;
            for (auto const& [name, accessorIndex] : gltfPrimitive.attributes)
            {
                tinygltf::Accessor const& gltfAccessor = m_gltfModel->accessors[static_cast<std::size_t>(accessorIndex)];
                tinygltf::BufferView const& gltfBufferView = m_gltfModel->bufferViews[static_cast<std::size_t>(gltfAccessor.bufferView)];

                vkgfx::BufferWithOffset& attributeBuffer = mesh.vertexBuffers.emplace_back();
                attributeBuffer.buffer = m_gltfResources->buffers[static_cast<std::size_t>(gltfBufferView.buffer)];
                attributeBuffer.offset = gltfBufferView.byteOffset + gltfAccessor.byteOffset; // TODO can be improved

                if (name == "COLOR_0")
                    demoMesh.metadata.attributeSemanticsConfig.hasColor = true;
                if (name == "TEXCOORD_0")
                    demoMesh.metadata.attributeSemanticsConfig.hasUv = true;
                if (name == "NORMAL")
                    demoMesh.metadata.attributeSemanticsConfig.hasNormal = true;
                if (name == "TANGENT")
                    demoMesh.metadata.attributeSemanticsConfig.hasTangent = true;

                vkgfx::AttributeType attributeType = findAttributeType2(gltfAccessor.type, gltfAccessor.componentType);

                std::size_t stride = gltfBufferView.byteStride;
                if (stride == 0)
                    stride = getAttributeByteSize(attributeType);

                vkgfx::VertexConfiguration::Binding& bindingConfig = demoMesh.metadata.vertexConfig.bindings.emplace_back();
                bindingConfig.stride = stride;

                vkgfx::VertexConfiguration::Attribute& attributeConfig = demoMesh.metadata.vertexConfig.attributes.emplace_back();
                attributeConfig.binding = attributeIndex;
                attributeConfig.location = findAttributeLocation(name);
                attributeConfig.offset = 0; // TODO can be improved
                attributeConfig.type = attributeType;

                // TODO implement
                assert(gltfPrimitive.mode == TINYGLTF_MODE_TRIANGLES);
                demoMesh.metadata.vertexConfig.topology = vkgfx::VertexTopology::Triangles;

                demoMesh.metadata.materialIndex = gltfPrimitive.material; // TODO check

                attributeIndex++;
            }

            demoMesh.handle = resourceManager.createMesh(std::move(mesh));
        }
    }

    {
        std::size_t const sceneIndex = static_cast<std::size_t>(m_gltfModel->defaultScene);
        tinygltf::Scene const& gltfScene = m_gltfModel->scenes[sceneIndex];
        auto demoScene = createDemoScene(*m_gltfModel, gltfScene);

        std::sort(demoScene.objects.begin(), demoScene.objects.end(), [](vkgfx::TestObject const& lhs, vkgfx::TestObject const& rhs)
        {
            if (lhs.pipeline != rhs.pipeline)
                return lhs.pipeline < rhs.pipeline;

            return lhs.material < rhs.material;
        });

        if (m_renderer)
        {
            for (auto const& demoObject : demoScene.objects)
                m_renderer->addTestObject(demoObject);
        }
    }

    m_currentScenePath = gltfPath;

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

    if (m_renderer)
    {
        m_services.debugDraw().draw(*m_renderer);
        m_renderer->draw();
    }

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

    updateMaterials();
    updateUI(m_lastFrameTime);
    updateScene(dt);
    updateCamera(dt);

    m_renderer->setCameraTransform(m_cameraTransform);
    m_renderer->setCameraParameters(m_cameraParameters);
    m_renderer->setLightParameters(m_lightParameters);

    m_imGuiDrawer->draw();
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

void DemoApplication::updateMaterials()
{
    std::vector<bool> imageReadyFlags;
    {
        std::lock_guard<std::mutex> guard{ m_imageReadyFlagsMutex };
        if (!m_imageReadyFlagsChanged)
            return;
        imageReadyFlags = m_imageReadyFlags;
        m_imageReadyFlagsChanged = false;
    }

    vkgfx::ResourceManager& resourceManager = m_renderer->getResourceManager();

    bool isAnyImageLoading = false;

    for (std::size_t i = 0; i < m_gltfModel->images.size(); i++)
    {
        tinygltf::Image const& gltfImage = m_gltfModel->images[i];

        if (!imageReadyFlags[i])
        {
            isAnyImageLoading = true;
            continue;
        }

        assert(!gltfImage.as_is);

        if (m_gltfResources->images[i])
            continue;

        uint32_t width = static_cast<uint32_t>(gltfImage.width);
        uint32_t height = static_cast<uint32_t>(gltfImage.height);
        std::size_t bitsPerComponent = static_cast<std::size_t>(gltfImage.bits);
        std::size_t components = static_cast<std::size_t>(gltfImage.component);

        vkgfx::ImageMetadata metadata;
        metadata.width = width;
        metadata.height = height;
        if (bitsPerComponent == 8 && components == 4)
            metadata.format = vkgfx::ImageFormat::R8G8B8A8;
        else
            assert(false);

        auto handle = resourceManager.createImage(metadata);
        resourceManager.uploadImage(handle, gltfImage.image);
        m_gltfResources->images[i] = handle;
    }
    
    for (std::size_t i = 0; i < m_gltfModel->textures.size(); i++)
    {
        tinygltf::Texture const& gltfTexture = m_gltfModel->textures[i];

        std::size_t const imageIndex = static_cast<std::size_t>(gltfTexture.source);

        vkgfx::ImageHandle imageHandle = m_gltfResources->images[imageIndex];

        if (!imageHandle)
            continue;

        vkgfx::Texture texture;
        texture.image = imageHandle;
        texture.sampler = m_defaultSampler;
        if (gltfTexture.sampler >= 0)
        {
            std::size_t const samplerIndex = static_cast<std::size_t>(gltfTexture.sampler);
            texture.sampler = m_gltfResources->samplers[samplerIndex];
        }

        vkgfx::TextureHandle handle = resourceManager.createTexture(std::move(texture));
        m_gltfResources->textures[i] = handle;
    }

    for (std::size_t i = 0; i < m_gltfModel->materials.size(); i++)
    {
        tinygltf::Material const& gltfMaterial = m_gltfModel->materials[i];

        vkgfx::MaterialHandle handle = m_gltfResources->materials[i].handle;
        vkgfx::Material* material = resourceManager.getMaterial(handle);
        assert(material);

        tinygltf::PbrMetallicRoughness const& gltfRoughness = gltfMaterial.pbrMetallicRoughness;

        if (gltfRoughness.baseColorTexture.index >= 0)
        {
            std::size_t const textureIndex = static_cast<std::size_t>(gltfRoughness.baseColorTexture.index);
            auto textureHandle = m_gltfResources->textures[textureIndex];
            if (textureHandle)
                material->albedo = textureHandle;
        }

        if (gltfMaterial.normalTexture.index >= 0)
        {
            std::size_t const textureIndex = static_cast<std::size_t>(gltfMaterial.normalTexture.index);
            auto textureHandle = m_gltfResources->textures[textureIndex];
            if (textureHandle)
                material->normalMap = textureHandle;
        }
    }

    if (!isAnyImageLoading)
        m_gltfModel = {};
}
