#include "Instance.h"

namespace
{
    bool hasEveryOption(std::vector<char const*> const& availableOptions, std::vector<char const*> const& requestedOptions)
    {
        for (const auto& requestedOption : requestedOptions)
        {
            auto it = std::find_if(availableOptions.begin(), availableOptions.end(), [requestedOption](char const* availableOption)
            {
                return std::strcmp(availableOption, requestedOption) == 0;
            });

            if (it == availableOptions.end())
                return false;
        }

        return true;
    }

    std::vector<VkLayerProperties> getAvailableLayers()
    {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> availableLayers(count);
        vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

        return availableLayers;
    }

    std::vector<VkExtensionProperties> getAvailableExtensions()
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

        return availableExtensions;
    }
}

vkr::Instance::Instance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump)
{
    m_availableLayers = getAvailableLayers();
    m_availableLayerNames.reserve(m_availableLayers.size());
    for (const auto& layer : m_availableLayers)
        m_availableLayerNames.push_back(layer.layerName);

    m_availableExtensions = getAvailableExtensions();
    m_availableExtensionNames.reserve(m_availableExtensions.size());
    for (const auto& extension : m_availableExtensions)
        m_availableExtensionNames.push_back(extension.extensionName);

    std::vector<char const*> requestedLayers;
    if (enableValidation)
        requestedLayers.push_back("VK_LAYER_KHRONOS_validation");
    if (enableApiDump)
        requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");

    std::vector<char const*> requestedExtensions = extensions;

    if (!hasEveryOption(m_availableLayerNames, requestedLayers))
        throw std::runtime_error("Some of the required validation layers aren't supported");

    if (!hasEveryOption(m_availableExtensionNames, requestedExtensions))
        throw std::runtime_error("Some of the required extensions aren't supported");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = requestedExtensions.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requestedLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = requestedLayers.data();

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
}

vkr::Instance::~Instance()
{
    vkDestroyInstance(m_handle, nullptr);
}
