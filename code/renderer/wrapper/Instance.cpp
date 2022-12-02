#include "Instance.h"

#include "Assert.h"
#include "PhysicalDevice.h"
#include "common/Utils.h"
#include "DebugMessage.h"

namespace
{
    nstl::vector<VkLayerProperties> getAvailableLayers()
    {
        uint32_t count = 0;
        VKO_ASSERT(vkEnumerateInstanceLayerProperties(&count, nullptr));
        nstl::vector<VkLayerProperties> availableLayers(count);
        VKO_ASSERT(vkEnumerateInstanceLayerProperties(&count, availableLayers.data()));

        return availableLayers;
    }

    nstl::vector<VkExtensionProperties> getAvailableExtensions()
    {
        uint32_t count = 0;
        VKO_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        nstl::vector<VkExtensionProperties> availableExtensions(count);
        VKO_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data()));

        return availableExtensions;
    }

    vko::DebugMessage::Level convertLevel(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
    {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            return vko::DebugMessage::Level::Warning;
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            return vko::DebugMessage::Level::Error;

        return vko::DebugMessage::Level::Info;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
    {
        vko::Instance* self = static_cast<vko::Instance*>(userData);

        vko::DebugMessage message;
        message.level = convertLevel(severity);
        if (callbackData->pMessageIdName)
            message.id = callbackData->pMessageIdName;
        if (callbackData->pMessage)
            message.text = callbackData->pMessage;

        self->dispatchDebugMessage(nstl::move(message));

        return VK_FALSE;
    }
}

vko::Instance::Instance(char const* appName, nstl::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump, nstl::function<void(DebugMessage)> onDebugMessage) : m_onDebugMessage(nstl::move(onDebugMessage))
{
    m_availableLayers = getAvailableLayers();
    m_availableLayerNames.reserve(m_availableLayers.size());
    for (const auto& layer : m_availableLayers)
        m_availableLayerNames.push_back(layer.layerName);

    m_availableExtensions = getAvailableExtensions();
    m_availableExtensionNames.reserve(m_availableExtensions.size());
    for (const auto& extension : m_availableExtensions)
        m_availableExtensionNames.push_back(extension.extensionName);

    createInstance(appName, extensions, enableValidation, enableApiDump);
    findFunctions();
    createDebugMessenger();
}

vko::Instance::~Instance()
{
    if (m_vkDestroyDebugUtilsMessengerEXT && m_debugMessenger)
        m_vkDestroyDebugUtilsMessengerEXT(m_handle.get(), m_debugMessenger.get(), nullptr);

    vkDestroyInstance(m_handle, nullptr);
}

void vko::Instance::createInstance(char const* appName, nstl::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump)
{
    nstl::vector<char const*> requestedLayers;
    if (enableValidation)
        requestedLayers.push_back("VK_LAYER_KHRONOS_validation");
    if (enableApiDump)
        requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");

    assert(vkc::utils::hasEveryOption(m_availableLayerNames, requestedLayers));
    assert(vkc::utils::hasEveryOption(m_availableExtensionNames, extensions));

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine"; // TODO change
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requestedLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = requestedLayers.data();

    VKO_ASSERT(vkCreateInstance(&instanceCreateInfo, nullptr, &m_handle.get()));
}

void vko::Instance::findFunctions()
{
    auto findFunction = [this](auto& destination, char const* name)
    {
        destination = reinterpret_cast<std::decay_t<decltype(destination)>>(vkGetInstanceProcAddr(m_handle.get(), name));
        assert(destination);
    };

    findFunction(m_vkCreateDebugUtilsMessengerEXT, "vkCreateDebugUtilsMessengerEXT");
    findFunction(m_vkDestroyDebugUtilsMessengerEXT, "vkDestroyDebugUtilsMessengerEXT");
    findFunction(m_vkSetDebugUtilsObjectNameEXT, "vkSetDebugUtilsObjectNameEXT");
}

void vko::Instance::createDebugMessenger()
{
    if (!m_vkCreateDebugUtilsMessengerEXT || !m_onDebugMessage)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugMessageCallback;
    createInfo.pUserData = this;

    VKO_ASSERT(m_vkCreateDebugUtilsMessengerEXT(m_handle.get(), &createInfo, nullptr, &m_debugMessenger.get()));
}

void vko::Instance::setDebugName(VkDevice device, uint64_t handle, VkObjectType type, char const* name) const
{
    if (!m_vkSetDebugUtilsObjectNameEXT)
        return;

    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = type;
    nameInfo.objectHandle = handle;
    nameInfo.pObjectName = name;

    VKO_ASSERT(m_vkSetDebugUtilsObjectNameEXT(device, &nameInfo));
}

nstl::vector<vko::PhysicalDevice> vko::Instance::findPhysicalDevices()
{
    uint32_t count = 0;
    VKO_ASSERT(vkEnumeratePhysicalDevices(m_handle, &count, nullptr));
    assert(count > 0);

    nstl::vector<VkPhysicalDevice> physicalDeviceHandles(count);
    VKO_ASSERT(vkEnumeratePhysicalDevices(m_handle, &count, physicalDeviceHandles.data()));

    nstl::vector<vko::PhysicalDevice> physicalDevices;
    physicalDevices.reserve(count);
    for (auto const& handle : physicalDeviceHandles)
        physicalDevices.emplace_back(handle);

    return physicalDevices;
}

void vko::Instance::dispatchDebugMessage(DebugMessage message)
{
    m_onDebugMessage(nstl::move(message));
}
