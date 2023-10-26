#include "instance.h"

#include "gfx_vk/config.h"
#include "gfx_vk/surface_factory.h"

#include "utils.h"

#include "gfx/resources.h"

#include "common/Utils.h" // TODO remove

#include "logging/logging.h"

#include "memory/memory.h"
#include "memory/tracking.h"

#include "nstl/array.h"
#include "nstl/vector.h"

namespace
{
    static auto scope_id = memory::tracking::create_scope_id("Rendering/Vulkan/Driver");

    void* allocate(void*, size_t size, [[maybe_unused]] size_t alignment, VkSystemAllocationScope)
    {
        MEMORY_TRACKING_SCOPE(scope_id);

        // TODO make use of alignment
        assert(alignment <= alignof(max_align_t));

        return memory::allocate(size);
    }

    void* reallocate(void*, void* ptr, size_t size, [[maybe_unused]] size_t alignment, VkSystemAllocationScope)
    {
        MEMORY_TRACKING_SCOPE(scope_id);

        // TODO make use of alignment
        assert(alignment <= alignof(max_align_t));

        return memory::reallocate(ptr, size);
    }

    void deallocate(void*, void* ptr)
    {
        MEMORY_TRACKING_SCOPE(scope_id);

        return memory::deallocate(ptr);
    }

    VkAllocationCallbacks create_allocator()
    {
        return VkAllocationCallbacks{
            .pUserData = nullptr,
            .pfnAllocation = &allocate,
            .pfnReallocation = &reallocate,
            .pfnFree = &deallocate,
        };
    }

    nstl::span<char const* const> get_device_extensions()
    {
        static nstl::vector<char const*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        return extensions;
    }

    void fill_physical_device_properties(gfx_vk::physical_device_properties& props, VkPhysicalDevice handle, VkSurfaceKHR surface)
    {
        uint32_t count = 0;

        vkGetPhysicalDeviceProperties(handle, &props.properties);

        nstl::vector<VkQueueFamilyProperties> queue_family_properties;
        vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, nullptr);
        queue_family_properties.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, queue_family_properties.data());

        GFX_VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, &props.capabilities));

        GFX_VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, nullptr));
        props.formats.resize(count);
        GFX_VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, props.formats.data()));

        GFX_VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &count, nullptr));
        props.present_modes.resize(count);
        GFX_VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &count, props.present_modes.data()));

        for (uint32_t familyIndex = 0; familyIndex < queue_family_properties.size(); familyIndex++)
        {
            VkBool32 hasPresentationSupport = false;
            GFX_VK_VERIFY(vkGetPhysicalDeviceSurfaceSupportKHR(handle, familyIndex, surface, &hasPresentationSupport));

            // TODO choose transfer queue in a smarter way
            if (queue_family_properties[familyIndex].queueFlags & VK_QUEUE_TRANSFER_BIT)
                props.transfer_queue_family = familyIndex;

            if (queue_family_properties[familyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                props.graphics_queue_family = familyIndex;

            if (hasPresentationSupport)
                props.present_queue_family = familyIndex;

            if (props.transfer_queue_family && props.graphics_queue_family && props.present_queue_family)
                break;
        }

        vkGetPhysicalDeviceMemoryProperties(handle, &props.memory_properties);
    }

    gfx::debug_message_level get_level(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
    {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            return gfx::debug_message_level::error;
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            return gfx::debug_message_level::warning;
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            return gfx::debug_message_level::info;
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            return gfx::debug_message_level::verbose;

        assert(false);
        return gfx::debug_message_level::error;
    }

    void on_debug_message(gfx::debug_message_level level, nstl::string_view, nstl::string_view text)
    {
        if (level == gfx::debug_message_level::verbose)
            logging::info("{}", text);
        if (level == gfx::debug_message_level::info)
            logging::info("{}", text);
        if (level == gfx::debug_message_level::warning)
            logging::warn("{}", text);
        if (level == gfx::debug_message_level::error)
            logging::error("{}", text);

        assert(level != gfx::debug_message_level::error);
    }
}

gfx_vk::instance::instance(surface_factory& factory, config const& config)
    : m_allocator(create_allocator())
{
    create_instance(factory, config);
    load_debug_functions(config);
    setup_debug_callback(config);
    GFX_VK_VERIFY(factory.create(m_instance, &m_allocator, &m_surface.get()));
    find_physical_device();
    create_device();
}

gfx_vk::instance::~instance()
{
    vkDestroyDevice(m_device, &m_allocator);
    vkDestroySurfaceKHR(m_instance, m_surface, &m_allocator);
    if (m_debug_messenger != VK_NULL_HANDLE)
        m_vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, &m_allocator);
    vkDestroyInstance(m_instance, &m_allocator);
}

void gfx_vk::instance::wait_idle()
{
    GFX_VK_VERIFY(vkDeviceWaitIdle(m_device));
}

void gfx_vk::instance::on_surface_changed()
{
    GFX_VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &m_physical_device_props.capabilities));
}

void gfx_vk::instance::create_instance(surface_factory& factory, config const& config)
{
    nstl::vector<char const*> enabled_layers;
    if (config.enable_validation)
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");

    nstl::vector<const char*> enabled_extensions;
    for (char const* surface_extension : factory.get_instance_extensions())
        enabled_extensions.push_back(surface_extension);
    if (config.enable_validation)
        enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    uint32_t count = 0;

    GFX_VK_VERIFY(vkEnumerateInstanceLayerProperties(&count, nullptr));
    nstl::vector<VkLayerProperties> available_layers{ count };
    GFX_VK_VERIFY(vkEnumerateInstanceLayerProperties(&count, available_layers.data()));

    GFX_VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    nstl::vector<VkExtensionProperties> available_extensions{ count };
    GFX_VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &count, available_extensions.data()));

    assert(vkc::utils::hasEveryOption(available_layers, enabled_layers, [](VkLayerProperties const& props) { return props.layerName; }));
    assert(vkc::utils::hasEveryOption(available_extensions, enabled_extensions, [](VkExtensionProperties const& props) { return props.extensionName; }));

    VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = config.name,
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .pEngineName = "No Engine", // TODO change
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .apiVersion = VK_API_VERSION_1_3,
    };

    VkInstanceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
        .ppEnabledLayerNames = enabled_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
        .ppEnabledExtensionNames = enabled_extensions.data(),
    };

    GFX_VK_VERIFY(vkCreateInstance(&info, &m_allocator, &m_instance.get()));
}

void gfx_vk::instance::load_debug_functions(config const& config)
{
    if (!config.enable_validation)
        return;

    auto load_function = [this](auto& destination, char const* name)
    {
        destination = reinterpret_cast<nstl::simple_decay_t<decltype(destination)>>(vkGetInstanceProcAddr(m_instance, name));
        assert(destination);
    };

    load_function(m_vkCreateDebugUtilsMessengerEXT, "vkCreateDebugUtilsMessengerEXT");
    load_function(m_vkDestroyDebugUtilsMessengerEXT, "vkDestroyDebugUtilsMessengerEXT");
    load_function(m_vkSetDebugUtilsObjectNameEXT, "vkSetDebugUtilsObjectNameEXT");
}

void gfx_vk::instance::setup_debug_callback(config const& config)
{
    if (!config.enable_validation)
        return;

    if (!m_vkCreateDebugUtilsMessengerEXT)
        return;

    auto callback = [](VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*) -> VkBool32
    {
        on_debug_message(get_level(severity), data->pMessageIdName, data->pMessage); // TODO expose the callback to the outside
        return VK_FALSE;
    };

    VkDebugUtilsMessengerCreateInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = callback;
    info.pUserData = this;

    GFX_VK_VERIFY(m_vkCreateDebugUtilsMessengerEXT(m_instance, &info, &m_allocator, &m_debug_messenger.get()));
}

void gfx_vk::instance::find_physical_device()
{
    uint32_t count = 0;
    GFX_VK_VERIFY(vkEnumeratePhysicalDevices(m_instance, &count, nullptr));
    nstl::vector<VkPhysicalDevice> handles{ count };
    GFX_VK_VERIFY(vkEnumeratePhysicalDevices(m_instance, &count, handles.data()));

    for (VkPhysicalDevice handle : handles)
    {
        nstl::vector<VkExtensionProperties> extensions;

        gfx_vk::physical_device_properties props;
        fill_physical_device_properties(props, handle, m_surface);

        GFX_VK_VERIFY(vkEnumerateDeviceExtensionProperties(handle, nullptr, &count, nullptr));
        extensions.resize(count);
        GFX_VK_VERIFY(vkEnumerateDeviceExtensionProperties(handle, nullptr, &count, extensions.data()));
        bool are_extensions_supported = vkc::utils::hasEveryOption(extensions, get_device_extensions(), [](VkExtensionProperties const& props) { return props.extensionName; });

        if (!are_extensions_supported)
            continue;

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(handle, &features);
        if (!features.samplerAnisotropy)
            continue;

        if (!props.transfer_queue_family || !props.graphics_queue_family || !props.present_queue_family)
            continue;
        if (props.formats.empty())
            continue;
        if (props.present_modes.empty())
            continue;

        m_physical_device = handle;
        m_physical_device_props = nstl::move(props);
        return;
    }

    assert(false);
}

void gfx_vk::instance::create_device()
{
    // TODO rewrite
    nstl::vector<uint32_t> queue_families = { *m_physical_device_props.transfer_queue_family };
    if (m_physical_device_props.graphics_queue_family != m_physical_device_props.transfer_queue_family)
        queue_families.push_back(*m_physical_device_props.graphics_queue_family);
    if (m_physical_device_props.present_queue_family != m_physical_device_props.transfer_queue_family && m_physical_device_props.present_queue_family != m_physical_device_props.graphics_queue_family)
        queue_families.push_back(*m_physical_device_props.present_queue_family);

    nstl::array priorities = { 1.0f };

    nstl::vector<VkDeviceQueueCreateInfo> queue_infos;
    queue_infos.reserve(queue_families.size());
    for (uint32_t queue_family : queue_families)
    {
        VkDeviceQueueCreateInfo& queueCreateInfo = queue_infos.emplace_back();
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queue_family;
        queueCreateInfo.queueCount = static_cast<uint32_t>(priorities.size());
        queueCreateInfo.pQueuePriorities = priorities.data();
    }

    VkPhysicalDeviceFeatures features{
        .geometryShader = VK_TRUE,
        .fillModeNonSolid = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
    };

    VkPhysicalDeviceIndexTypeUint8FeaturesEXT feature_uint8{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT,
        .indexTypeUint8 = true,
    };

    VkDeviceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &feature_uint8,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
        .pQueueCreateInfos = queue_infos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(get_device_extensions().size()),
        .ppEnabledExtensionNames = get_device_extensions().data(),
        .pEnabledFeatures = &features,
    };

    GFX_VK_VERIFY(vkCreateDevice(m_physical_device, &info, &m_allocator, &m_device.get()));

    for (uint32_t queue_family : queue_families)
    {
        VkQueue handle = VK_NULL_HANDLE;
        vkGetDeviceQueue(m_device, queue_family, 0, &handle);

        if (queue_family == m_physical_device_props.transfer_queue_family)
            m_transfer_queue = handle;
        if (queue_family == m_physical_device_props.graphics_queue_family)
            m_graphics_queue = handle;
        if (queue_family == m_physical_device_props.present_queue_family)
            m_present_queue = handle;
    }

    assert(m_graphics_queue != VK_NULL_HANDLE && m_present_queue != VK_NULL_HANDLE);
}

void gfx_vk::instance::set_debug_name_v(uint64_t handle, VkObjectType type, nstl::string_view format, picofmt::args_list const& args)
{
    if (!m_vkSetDebugUtilsObjectNameEXT)
        return;

    nstl::string name = common::vformat(format, args);

    VkDebugUtilsObjectNameInfoEXT info{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name.c_str(),
    };

    GFX_VK_VERIFY(m_vkSetDebugUtilsObjectNameEXT(m_device, &info));
}
