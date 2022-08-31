#include "DemoApplication.h"

#include <memory>
#include <array>
#include <vector>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "tiny_gltf.h"
#include "glm.h"
#include "magic_enum.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#pragma warning(push, 0)
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#pragma warning(pop)

#include "Application.h"
#include "wrapper/DescriptorPool.h"
#include "wrapper/Device.h"
#include "wrapper/Instance.h" // for imgui
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Pipeline.h" // imgui
#include "wrapper/Queue.h" // imgui
#include "Renderer.h"
#include "wrapper/RenderPass.h" // imgui
#include "wrapper/Sampler.h"
#include "Drawable.h"
#include "ScopedOneTimeCommandBuffer.h" // imgui
#include "wrapper/ShaderModule.h"
#include "wrapper/Swapchain.h" // imgui
#include "Texture.h"
#include "wrapper/DebugMessage.h"
#include "BufferWithMemory.h"

#include "ScopedDebugCommands.h"
#include "SceneObject.h"
#include "ShaderPackage.h"

#include "vkgfx/ResourceManager.h"
#include "vkgfx/Image.h" // TODO don't include it
#include "vkgfx/Buffer.h" // TODO don't include it
#include "vkgfx/ShaderModule.h" // TODO don't include it

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
    bool const API_DUMP_ENABLED = false;

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

    std::shared_ptr<tinygltf::Model> loadModel(std::string const& path)
    {
        std::shared_ptr<tinygltf::Model> model = std::make_shared<tinygltf::Model>();
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        bool success = loader.LoadASCIIFromFile(model.get(), &err, &warn, path);

        if (!warn.empty())
            std::cout << warn << std::endl;
        if (!err.empty())
            std::cout << err << std::endl;

        if (!success)
            return nullptr;

        return model;
    }

    vkr::VertexLayout::ComponentType findComponentType(int gltfComponentType)
    {
        switch (gltfComponentType)
        {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            return vkr::VertexLayout::ComponentType::Byte;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return vkr::VertexLayout::ComponentType::UnsignedByte;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            return vkr::VertexLayout::ComponentType::Short;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return vkr::VertexLayout::ComponentType::UnsignedShort;
        case TINYGLTF_COMPONENT_TYPE_INT:
            return vkr::VertexLayout::ComponentType::Int;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return vkr::VertexLayout::ComponentType::UnsignedInt;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return vkr::VertexLayout::ComponentType::Float;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return vkr::VertexLayout::ComponentType::Double;
        }

        throw std::invalid_argument("gltfComponentType");
    }

    vkr::VertexLayout::AttributeType findAttributeType(int gltfAttributeType)
    {
        switch (gltfAttributeType)
        {
        case TINYGLTF_TYPE_VEC2:
            return vkr::VertexLayout::AttributeType::Vec2;
        case TINYGLTF_TYPE_VEC3:
            return vkr::VertexLayout::AttributeType::Vec3;
        case TINYGLTF_TYPE_VEC4:
            return vkr::VertexLayout::AttributeType::Vec4;
        case TINYGLTF_TYPE_MAT2:
            return vkr::VertexLayout::AttributeType::Mat2;
        case TINYGLTF_TYPE_MAT3:
            return vkr::VertexLayout::AttributeType::Mat3;
        case TINYGLTF_TYPE_MAT4:
            return vkr::VertexLayout::AttributeType::Mat4;
        }

        throw std::invalid_argument("gltfAttributeType");
    }

    std::size_t getNumberOfComponents(vkr::VertexLayout::AttributeType type)
    {
        switch (type)
        {
        case vkr::VertexLayout::AttributeType::Vec2:
            return 2;
        case vkr::VertexLayout::AttributeType::Vec3:
            return 3;
        case vkr::VertexLayout::AttributeType::Vec4:
            return 4;
        case vkr::VertexLayout::AttributeType::Mat2:
            return 4;
        case vkr::VertexLayout::AttributeType::Mat3:
            return 9;
        case vkr::VertexLayout::AttributeType::Mat4:
            return 16;
        }

        throw std::invalid_argument("type");
    }

    std::size_t getComponentByteSize(vkr::VertexLayout::ComponentType type)
    {
        switch (type)
        {
        case vkr::VertexLayout::ComponentType::Byte:
            return sizeof(int8_t);
        case vkr::VertexLayout::ComponentType::UnsignedByte:
            return sizeof(uint8_t);
        case vkr::VertexLayout::ComponentType::Short:
            return sizeof(int16_t);
        case vkr::VertexLayout::ComponentType::UnsignedShort:
            return sizeof(uint16_t);
        case vkr::VertexLayout::ComponentType::Int:
            return sizeof(int32_t);
        case vkr::VertexLayout::ComponentType::UnsignedInt:
            return sizeof(uint32_t);
        case vkr::VertexLayout::ComponentType::Float:
            return sizeof(float);
        case vkr::VertexLayout::ComponentType::Double:
            return sizeof(double);
        }

        throw std::invalid_argument("type");
    }

    std::size_t getAttributeStride(vkr::VertexLayout::AttributeType attributeType, vkr::VertexLayout::ComponentType componentType)
    {
        return getNumberOfComponents(attributeType) * getComponentByteSize(componentType);
    }

    std::size_t findAttributeLocation(std::string const& name)
    {
        static std::vector<std::string> const attributeNames = { "POSITION", "COLOR_0", "TEXCOORD_0", "NORMAL", "TANGENT" }; // TODO move to the shader metadata

        auto it = std::find(attributeNames.cbegin(), attributeNames.cend(), name);

        if (it != attributeNames.end())
            return static_cast<std::size_t>(std::distance(attributeNames.begin(), it));

        throw std::runtime_error("Unkown attribute name: " + name);
    }

    std::unique_ptr<vkr::Mesh> createMesh(vkr::Application const& app, std::shared_ptr<tinygltf::Model> const& model, tinygltf::Primitive const& primitive, GltfVkResources const& resources)
    {
        vkr::VertexLayout layout;
        vkr::Mesh::Metadata metadata;

        int bufferIndex = 0;

        {
            tinygltf::Accessor const& indexAccessor = model->accessors[static_cast<std::size_t>(primitive.indices)];
            tinygltf::BufferView const& indexBufferView = model->bufferViews[static_cast<std::size_t>(indexAccessor.bufferView)];
            layout.setIndexType(findComponentType(indexAccessor.componentType));
            layout.setIndexDataOffset(indexBufferView.byteOffset + indexAccessor.byteOffset);
            layout.setIndexCount(indexAccessor.count);

            bufferIndex = indexBufferView.buffer;
        }

        std::vector<vkr::VertexLayout::Binding> bindings;

        bindings.reserve(model->bufferViews.size());

        for (auto const& [name, accessorIndex] : primitive.attributes)
        {
            tinygltf::Accessor const& accessor = model->accessors[static_cast<std::size_t>(accessorIndex)];
            tinygltf::BufferView const& bufferView = model->bufferViews[static_cast<std::size_t>(accessor.bufferView)];

            assert(bufferIndex == bufferView.buffer);

            // TODO fix this hack
            if (name == "COLOR_0")
                metadata.hasColor = true;
            if (name == "TEXCOORD_0")
				metadata.hasTexCoord = true;
			if (name == "NORMAL")
				metadata.hasNormal = true;
			if (name == "TANGENT")
				metadata.hasTangent = true;

            std::size_t location = findAttributeLocation(name);
            vkr::VertexLayout::AttributeType attributeType = findAttributeType(accessor.type);
            vkr::VertexLayout::ComponentType componentType = findComponentType(accessor.componentType);
            std::size_t offset = accessor.byteOffset;

			// TODO validate if attribute & component types match the attribute semantics

            std::size_t stride = bufferView.byteStride;
            if (stride == 0)
                stride = getAttributeStride(attributeType, componentType);

            bindings.emplace_back(bufferView.byteOffset + offset, stride)
                .addAttribute(location, attributeType, componentType, 0);
        }

        layout.setBindings(std::move(bindings));

        assert(bufferIndex >= 0 && bufferIndex < resources.buffers.size());

        vkr::BufferWithMemory& buffer = *resources.buffers[static_cast<std::size_t>(bufferIndex)];

        return std::make_unique<vkr::Mesh>(app, buffer.buffer(), std::move(layout), std::move(metadata));
    }

    std::vector<unsigned char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        std::streamsize fileSize = file.tellg();
        std::vector<unsigned char> buffer(static_cast<std::size_t>(fileSize));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize); // TODO check if okay
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

    m_commands["camera.znear"] = coil::bindProperty(&DemoApplication::getCameraNearZ, &DemoApplication::setCameraNearZ, this);
    m_commands["camera.zfar"] = coil::bindProperty(&DemoApplication::getCameraFarZ, &DemoApplication::setCameraFarZ, this);
    m_commands["camera.speed"] = coil::variable(&m_cameraSpeed);
    m_commands["camera.mouse_sensitivity"] = coil::variable(&m_mouseSensitivity);
    m_commands["camera.pos"] = coil::bindProperty(&DemoApplication::getCameraPos, &DemoApplication::setCameraPos, this);
    m_commands["camera.angles"] = coil::property([this]() {
        return glm::degrees(glm::eulerAngles(getCameraRotation()));
    }, [this](glm::vec3 const& angles) {
        setCameraRotation(createRotation(angles));
    });

    m_commands["light.pos"] = coil::property([this]() {
        return m_light->getTransform().getWorldPos();
    }, [this](glm::vec3 const& value) {
        m_light->getTransform().setWorldPos(value);
    });
    m_commands["light.color"] = coil::property([this]() {
        return m_light->getColor();
    }, [this](glm::vec3 const& value) {
        m_light->setColor(value);
    });
    m_commands["light.intensity"] = coil::property([this]() {
        return m_light->getIntensity();
    }, [this](float value) {
        m_light->setIntensity(value);
    });

    m_commands["fps"].description("Show/hide the FPS widget") = ::toggle(&m_showFps);
    m_commands["fps.update_period"].description("Update period of the FPS widget") = coil::variable(&m_fpsUpdatePeriod);

    m_keyState.resize(1 << 8 * sizeof(char), false);

    m_window = std::make_unique<vkr::GlfwWindow>(TARGET_WINDOW_WIDTH, TARGET_WINDOW_HEIGHT, "Vulkan Demo");
    m_window->addResizeCallback([this](int, int) { onFramebufferResized(); });
    m_window->addKeyCallback([this](vkr::GlfwWindow::Action action, vkr::GlfwWindow::Key key, char c, vkr::GlfwWindow::Modifiers modifiers) { onKey(action, key, c, modifiers); });
    m_window->addMouseMoveCallback([this](glm::vec2 const& delta) { onMouseMove(delta); });

    auto messageCallback = [](vko::DebugMessage m)
    {
		if (m.level == vko::DebugMessage::Level::Info)
			spdlog::info(m.text);
		if (m.level == vko::DebugMessage::Level::Warning)
			spdlog::warn(m.text);
		if (m.level == vko::DebugMessage::Level::Error)
			spdlog::error(m.text);

        assert(m.level != vko::DebugMessage::Level::Error);
    };

    m_application = std::make_unique<vkr::Application>("Vulkan demo", VALIDATION_ENABLED, API_DUMP_ENABLED, *m_window, std::move(messageCallback));

    m_resourceManager = std::make_unique<vkgfx::ResourceManager>(m_application->getDevice(), m_application->getPhysicalDevice(), m_application->getShortLivedCommandPool(), m_application->getDevice().getGraphicsQueue());

    m_renderer = std::make_unique<vkr::Renderer>(getApp());
    m_renderer->setWaitUntilWindowInForegroundCallback([this]() { m_window->waitUntilInForeground(); });

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

    loadScene("");
}

DemoApplication::~DemoApplication()
{
    unloadImgui();

    // TODO move to the renderer
    getApp().getDevice().waitIdle();

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

void DemoApplication::loadImgui()
{
    if (ImGui::GetCurrentContext())
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    io.Fonts->AddFontDefault();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_window->getHandle(), true);

    m_imguiDescriptorPool = std::make_unique<vko::DescriptorPool>(getApp().getDevice());
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = getApp().getInstance().getHandle();
    init_info.PhysicalDevice = getApp().getPhysicalDevice().getHandle();
    init_info.Device = getApp().getDevice().getHandle();
    init_info.QueueFamily = getApp().getDevice().getGraphicsQueue().getFamily().getIndex();
    init_info.Queue = getApp().getDevice().getGraphicsQueue().getHandle();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_imguiDescriptorPool->getHandle();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = 2; // TODO fetch?
    init_info.ImageCount = static_cast<uint32_t>(m_renderer->getSwapchain().getImageCount());
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, m_renderer->getRenderPass().getHandle());

    {
        vkr::ScopedOneTimeCommandBuffer buffer{ getApp() };
        ImGui_ImplVulkan_CreateFontsTexture(buffer.getHandle());
        buffer.submit();

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void DemoApplication::unloadImgui()
{
    if (!ImGui::GetCurrentContext())
        return;

    getApp().getDevice().waitIdle();

    m_imguiDescriptorPool = nullptr;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DemoApplication::onFramebufferResized()
{
    m_renderer->onFramebufferResized();
    m_application->onSurfaceChanged();
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
    if (!m_activeCameraObject)
        return;

    glm::vec3 angleDelta = m_mouseSensitivity * glm::vec3{ -delta.y, -delta.x, 0.0f };
    glm::quat rotationDelta = createRotation(angleDelta);

    vkr::Transform& cameraTransform = m_activeCameraObject->getTransform();
    cameraTransform.setLocalRotation(cameraTransform.getLocalRotation() * rotationDelta);
}

std::shared_ptr<vkr::SceneObject> DemoApplication::addSceneObjectsFromNode(std::shared_ptr<tinygltf::Model> const& model, tinygltf::Node const& node, Scene& scene)
{
    std::shared_ptr<vkr::SceneObject> object = std::make_shared<vkr::SceneObject>();

    object->getTransform().setLocalMatrix(createMatrix(node));
    scene.objects.push_back(object);

    if (node.mesh >= 0)
    {
        vkr::ShaderConfiguration shaderConfiguration;

        std::size_t const meshIndex = static_cast<std::size_t>(node.mesh);

        tinygltf::Mesh const& gltfMesh = model->meshes[meshIndex];

        for (tinygltf::Primitive const& gltfPrimitive : gltfMesh.primitives)
        {
            std::shared_ptr<vkr::SceneObject> subObject = std::make_shared<vkr::SceneObject>();
            scene.objects.push_back(subObject);
            object->getTransform().addChild(subObject->getTransform());

            // TODO don't create mesh if it can be shared with other nodes
            auto mesh = createMesh(getApp(), model, gltfPrimitive, *m_gltfResources);

            auto const& meshMetadata = mesh->getMetadata();
            shaderConfiguration.hasColor = meshMetadata.hasColor;
            shaderConfiguration.hasTexCoord = meshMetadata.hasTexCoord;
            shaderConfiguration.hasNormal = meshMetadata.hasNormal;
            shaderConfiguration.hasTangent = meshMetadata.hasTangent;

            std::size_t const materialIndex = static_cast<std::size_t>(gltfPrimitive.material);
            tinygltf::Material const& gltfMaterial = model->materials[materialIndex];

            tinygltf::PbrMetallicRoughness const& gltfRoughness = gltfMaterial.pbrMetallicRoughness;

            auto material = std::make_shared<vkr::Material>();
            material->setColor(createColor(gltfRoughness.baseColorFactor));

            shaderConfiguration.hasTexture = true; // TODO remove if always true
            if (gltfRoughness.baseColorTexture.index >= 0)
            {
                std::size_t const textureIndex = static_cast<std::size_t>(gltfRoughness.baseColorTexture.index);
                material->setTexture(m_gltfResources->textures[textureIndex]);
            }
            else
            {
                material->setTexture(m_fallbackAlbedo);
            }

            shaderConfiguration.hasNormalMap = true; // TODO remove if always true
            if (gltfMaterial.normalTexture.index >= 0)
            {
                std::size_t const textureIndex = static_cast<std::size_t>(gltfMaterial.normalTexture.index);
                material->setNormalMap(m_gltfResources->textures[textureIndex]);
            }
            else
            {
                material->setNormalMap(m_fallbackNormalMap);
            }

            material->setIsDoubleSided(gltfMaterial.doubleSided);

            std::string const* vertexShaderPath = m_defaultVertexShader->get(shaderConfiguration);
            std::string const* fragmentShaderPath = m_defaultFragmentShader->get(shaderConfiguration);

            if (!vertexShaderPath || !fragmentShaderPath)
                throw std::runtime_error("Failed to find the shader");

            auto shaderKey = vkr::Shader::Key{}
                .addStage(vko::ShaderModuleType::Vertex, *vertexShaderPath)
                .addStage(vko::ShaderModuleType::Fragment, *fragmentShaderPath);

            material->setShaderKey(std::move(shaderKey));

            auto drawable = std::make_unique<vkr::Drawable>(std::move(mesh), std::move(material));
            subObject->setDrawable(std::move(drawable));
        }
    }

    if (node.camera >= 0)
    {
        std::size_t const cameraIndex = static_cast<std::size_t>(node.camera);
        tinygltf::Camera const& gltfCamera = model->cameras[cameraIndex];

        if (gltfCamera.type == "perspective")
        {
            float const aspect = static_cast<float>(gltfCamera.perspective.aspectRatio);
            float const fov = static_cast<float>(gltfCamera.perspective.yfov);
            float const znear = static_cast<float>(gltfCamera.perspective.znear);
            float const zfar = static_cast<float>(gltfCamera.perspective.zfar);

            auto camera = std::make_shared<vkr::Camera>();
            camera->setAspect(aspect);
            camera->setFov(glm::degrees(fov));
            camera->setPlanes(znear, zfar);

            object->setCamera(camera);
        }
        else
        {
            throw std::runtime_error("Unknown camera type");
        }
    }

    return object;
}

std::shared_ptr<vkr::SceneObject> DemoApplication::createSceneObjectWithChildren(std::shared_ptr<tinygltf::Model> const& model, Scene& scene, std::size_t nodeIndex)
{
    tinygltf::Node const& node = model->nodes[nodeIndex];

    std::shared_ptr<vkr::SceneObject> parent = addSceneObjectsFromNode(model, node, scene);

    for (auto childNodeIndex : node.children)
    {
        std::shared_ptr<vkr::SceneObject> child = createSceneObjectWithChildren(model, scene, static_cast<std::size_t>(childNodeIndex));
        parent->getTransform().addChild(child->getTransform());
    }

    return parent;
}

Scene DemoApplication::createSceneObjectHierarchy(std::shared_ptr<tinygltf::Model> const& model)
{
    Scene scene;

    std::size_t const sceneIndex = static_cast<std::size_t>(model->defaultScene);
    tinygltf::Scene const& gltfScene = model->scenes[sceneIndex];

    for (std::size_t nodeIndex : gltfScene.nodes)
        createSceneObjectWithChildren(model, scene, nodeIndex);

    return scene;
}

void DemoApplication::clearScene()
{
    // move to the renderer
    getApp().getDevice().waitIdle();

    m_renderer->clearObjects();
    m_light = nullptr;
    m_activeCameraObject = nullptr;
    m_gltfResources = nullptr;
    m_defaultVertexShader = nullptr;
    m_defaultFragmentShader = nullptr;
}

bool DemoApplication::loadScene(std::string const& gltfPath)
{
    clearScene();

    m_defaultVertexShader = std::make_unique<vkr::ShaderPackage>("data/shaders/packaged/shader.vert");
    m_defaultFragmentShader = std::make_unique<vkr::ShaderPackage>("data/shaders/packaged/shader.frag");

    for (auto const& [configuration, modulePath] : m_defaultVertexShader->getAll())
    {
        m_resourceManager->createShaderModule(readFile(modulePath), vko::ShaderModuleType::Vertex, "main");
    }
    for (auto const& [configuration, modulePath] : m_defaultFragmentShader->getAll())
    {
        m_resourceManager->createShaderModule(readFile(modulePath), vko::ShaderModuleType::Fragment, "main");
    }

    m_fallbackSampler = std::make_shared<vko::Sampler>(getApp().getDevice());
    m_fallbackAlbedo = std::make_unique<vkr::Texture>(getApp(), std::array<unsigned char, 4>{ 0xff, 0xff, 0xff, 0xff }, 1, 1, 8, 4, m_fallbackSampler);
    m_fallbackNormalMap = std::make_unique<vkr::Texture>(getApp(), std::array<unsigned char, 4>{ 0x80, 0x80, 0xff, 0xff }, 1, 1, 8, 4, m_fallbackSampler);

    auto gltfModel = gltfPath.empty() ? nullptr : loadModel(gltfPath);

    if (!gltfPath.empty() && !gltfModel)
        return false;

    m_gltfResources = std::make_unique<GltfVkResources>();

    if (gltfModel)
    {
        for (auto const& buffer : gltfModel->buffers)
        {
            auto const& data = buffer.data;
            VkDeviceSize bufferSize = sizeof(data[0]) * data.size();
            void const* bufferData = data.data();

            vkr::BufferWithMemory buffer{ getApp().getDevice(), getApp().getPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

            vkr::BufferWithMemory stagingBuffer{ getApp().getDevice(), getApp().getPhysicalDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
            stagingBuffer.memory().copyFrom(bufferData, bufferSize);

            vkr::ScopedOneTimeCommandBuffer commandBuffer{ getApp() };
            vko::Buffer::copy(commandBuffer.getHandle(), stagingBuffer.buffer(), buffer.buffer());
            commandBuffer.submit();

            m_gltfResources->buffers.push_back(std::make_unique<vkr::BufferWithMemory>(std::move(buffer)));

            {
                auto handle = m_resourceManager->createBuffer(bufferSize);
                m_resourceManager->uploadBuffer(handle, data);
            }
        }

        for (tinygltf::Texture const& texture : gltfModel->textures)
        {
            std::size_t const samplerIndex = static_cast<std::size_t>(texture.sampler);
            tinygltf::Sampler const& gltfSampler = gltfModel->samplers[samplerIndex];

            auto convertFilterMode = [](int gltfMode)
            {
                switch (gltfMode)
                {
                case TINYGLTF_TEXTURE_FILTER_NEAREST:
                case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
                case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
                    return vko::Sampler::FilterMode::Nearest;
                case -1:
                case TINYGLTF_TEXTURE_FILTER_LINEAR:
                case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
                case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
                    return vko::Sampler::FilterMode::Linear;
                }

                throw std::invalid_argument("gltfMode");
            };

            auto convertWrapMode = [](int gltfMode)
            {
                switch (gltfMode)
                {
                case -1:
                case TINYGLTF_TEXTURE_WRAP_REPEAT:
                    return vko::Sampler::WrapMode::Repeat;
                case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                    return vko::Sampler::WrapMode::ClampToEdge;
                case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                    return vko::Sampler::WrapMode::Mirror;
                }

                throw std::invalid_argument("gltfMode");
            };

            auto magFilter = convertFilterMode(gltfSampler.magFilter);
            auto minFilter = convertFilterMode(gltfSampler.minFilter);
            auto wrapU = convertWrapMode(gltfSampler.wrapS);
            auto wrapV = convertWrapMode(gltfSampler.wrapT);

            auto sampler = std::make_shared<vko::Sampler>(getApp().getDevice(), magFilter, minFilter, wrapU, wrapV);

            std::size_t const imageIndex = static_cast<std::size_t>(texture.source);
            tinygltf::Image const& gltfImage = gltfModel->images[imageIndex];

            uint32_t width = static_cast<uint32_t>(gltfImage.width);
            uint32_t height = static_cast<uint32_t>(gltfImage.height);
            std::size_t bitsPerComponent = static_cast<std::size_t>(gltfImage.bits);
            std::size_t components = static_cast<std::size_t>(gltfImage.component);

            m_gltfResources->textures.push_back(std::make_shared<vkr::Texture>(getApp(), gltfImage.image, width, height, bitsPerComponent, components, sampler));

            {
                vkgfx::ImageMetadata metadata;
                metadata.width = width;
                metadata.height = height;
                if (bitsPerComponent == 8 && components == 4)
                    metadata.format = vkgfx::ImageFormat::R8G8B8A8;
                else
                    throw std::runtime_error("Not implemented");

                auto handle = m_resourceManager->createImage(metadata);
                m_resourceManager->uploadImage(handle, gltfImage.image);
            }
        }
    }

    if (gltfModel)
        m_scene = createSceneObjectHierarchy(gltfModel);

	auto defaultCameraObject = std::make_shared<vkr::SceneObject>();
	{
		defaultCameraObject->setCamera(std::make_shared<vkr::Camera>());

		vkr::Transform& cameraTransform = defaultCameraObject->getTransform();
		cameraTransform.setLocalPos(CAMERA_POS);
		cameraTransform.setLocalRotation(createRotation(CAMERA_ANGLES));
	}
    m_scene.objects.push_back(defaultCameraObject);

    for (auto const& object : m_scene.objects)
    {
        if (object->getDrawable())
            m_renderer->addDrawable(*object);

        if (!m_activeCameraObject && object->getCamera())
            m_activeCameraObject = object;
    }

    if (!m_activeCameraObject)
        m_activeCameraObject = defaultCameraObject;

    m_renderer->setCamera(m_activeCameraObject);

    m_light = std::make_shared<vkr::Light>();
    m_light->getTransform().setLocalPos(LIGHT_POS);
    m_light->setColor(LIGHT_COLOR);
    m_light->setIntensity(LIGHT_INTENSITY);
    m_renderer->setLight(m_light);

    m_currentScenePath = gltfPath;

    return true;
}

void DemoApplication::updateUI(float frameTime, float fenceTime)
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

    // TODO implement ImGui bindings manually
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_drawImguiDemo)
        ImGui::ShowDemoWindow(&m_drawImguiDemo);
    if (m_drawImguiDebugger)
        ImGui::ShowMetricsWindow(&m_drawImguiDebugger);
    if (m_drawImguiStyleEditor)
        ImGui::ShowStyleEditor();

    float cpuUtilizationRatio = frameTime > 0 ? 1.0f - fenceTime / frameTime : 1.0f;

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
        ImGui::Text("Fence time %.3f ms", fenceTime * 1000.0f);
        ImGui::Text("CPU Utilization %.2f%%", cpuUtilizationRatio * 100.0f);
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

    ImGui::Render();

    m_window->setCanCaptureCursor(!io.WantCaptureMouse);
}

void DemoApplication::drawFrame()
{
    update();
    m_services.debugDraw().draw(*m_renderer);
    m_renderer->draw();
    m_fpsDrawnFrames++;
}

void DemoApplication::update()
{
    if (m_frameTimer.getTime() > m_fpsUpdatePeriod)
    {
        float multiplier = 1.0f / static_cast<float>(m_fpsDrawnFrames);
        m_lastFrameTime = m_frameTimer.loop() * multiplier;
        m_lastFenceTime = m_renderer->getCumulativeFenceTime() * multiplier;
        m_renderer->resetCumulativeFenceTime();
        m_fpsDrawnFrames = 0;
    }

    float const dt = m_lastFrameTime;

    m_notifications->update(dt);

    updateUI(m_lastFrameTime, m_lastFenceTime);
    updateScene(dt);
    updateCamera(dt);
}

void DemoApplication::updateScene(float)
{
    m_services.debugDraw().box(m_light->getTransform().getWorldPos(), glm::identity<glm::quat>(), glm::vec3(0.1f), { 1.0f, 0.0f, 0.0f }, -1.0f);
}

void DemoApplication::updateCamera(float dt)
{
    if (!m_activeCameraObject)
        return;

    // TODO handle input properly
    if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
        return;

    vkr::Transform& cameraTransform = m_activeCameraObject->getTransform();

    glm::vec3 posDelta = glm::zero<glm::vec3>();

    if (m_keyState['A'])
        posDelta += -cameraTransform.getRightVector();
    if (m_keyState['D'])
        posDelta += cameraTransform.getRightVector();
    if (m_keyState['S'])
        posDelta += -cameraTransform.getForwardVector();
    if (m_keyState['W'])
        posDelta += cameraTransform.getForwardVector();
    if (m_keyState['Q'])
        posDelta += -cameraTransform.getUpVector();
    if (m_keyState['E'])
        posDelta += cameraTransform.getUpVector();

    if (glm::length2(posDelta) > glm::epsilon<float>())
        posDelta = glm::normalize(posDelta);

    cameraTransform.setWorldPos(cameraTransform.getWorldPos() + m_cameraSpeed * dt * posDelta);
}

glm::vec3 DemoApplication::getCameraPos() const
{
    return m_activeCameraObject->getTransform().getWorldPos();
}

void DemoApplication::setCameraPos(glm::vec3 const& pos)
{
    m_activeCameraObject->getTransform().setWorldPos(pos);
}

glm::quat DemoApplication::getCameraRotation() const
{
    return m_activeCameraObject->getTransform().getLocalRotation();
}

void DemoApplication::setCameraRotation(glm::quat const& rotation)
{
    m_activeCameraObject->getTransform().setLocalRotation(rotation);
}

float DemoApplication::getCameraNearZ() const
{
    return m_activeCameraObject->getCamera()->nearZ();
}

void DemoApplication::setCameraNearZ(float nearZ)
{
    m_activeCameraObject->getCamera()->setNearZ(nearZ);
}

float DemoApplication::getCameraFarZ() const
{
    return m_activeCameraObject->getCamera()->farZ();
}

void DemoApplication::setCameraFarZ(float farZ)
{
    m_activeCameraObject->getCamera()->setFarZ(farZ);
}
