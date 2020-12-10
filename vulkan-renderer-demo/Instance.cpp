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

    std::vector<char const*> getAvailableValidationLayers()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::vector<char const*> names;
        names.reserve(availableLayers.size());

        for (const auto& layer : availableLayers)
            names.push_back(layer.layerName);

        return names;
    }

    std::vector<char const*> getAvailableExtensions()
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

        std::vector<char const*> names;
        names.reserve(supportedExtensions.size());

        for (const auto& extension : supportedExtensions)
            names.push_back(extension.extensionName);

        return names;
    }
}

vkr::Instance::Instance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump)
{
    std::vector<char const*> requestedValidationLayers;
    if (enableValidation)
        requestedValidationLayers.push_back("VK_LAYER_KHRONOS_validation");
    if (enableApiDump)
        requestedValidationLayers.push_back("VK_LAYER_LUNARG_api_dump");

    std::vector<char const*> requestedExtensions = extensions;

    if (!hasEveryOption(getAvailableValidationLayers(), requestedValidationLayers))
        throw std::runtime_error("Some of the required validation layers aren't supported");

    if (!hasEveryOption(getAvailableExtensions(), requestedExtensions))
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
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requestedValidationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = requestedValidationLayers.data();

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
}

vkr::Instance::~Instance()
{
    vkDestroyInstance(m_handle, nullptr);
}
