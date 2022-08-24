#include "Instance.h"

#include <stdexcept>

#include "PhysicalDevice.h"
#include "Utils.h"

namespace
{
    std::vector<VkLayerProperties> getAvailableLayers()
    {
        uint32_t count = 0;
        VKR_ASSERT(vkEnumerateInstanceLayerProperties(&count, nullptr));
        std::vector<VkLayerProperties> availableLayers(count);
        VKR_ASSERT(vkEnumerateInstanceLayerProperties(&count, availableLayers.data()));

        return availableLayers;
    }

    std::vector<VkExtensionProperties> getAvailableExtensions()
    {
        uint32_t count = 0;
        VKR_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        std::vector<VkExtensionProperties> availableExtensions(count);
        VKR_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data()));

        return availableExtensions;
    }
}

vko::Instance::Instance(std::string const& appName, std::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump)
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
}

vko::Instance::~Instance()
{
    vkDestroyInstance(m_handle, nullptr);
}

void vko::Instance::createInstance(std::string const& appName, std::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump)
{
    std::vector<char const*> requestedLayers;
    if (enableValidation)
        requestedLayers.push_back("VK_LAYER_KHRONOS_validation");
    if (enableApiDump)
        requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");

    if (!vkr::utils::hasEveryOption(m_availableLayerNames, requestedLayers))
        throw std::runtime_error("Some of the required validation layers aren't supported");

    if (!vkr::utils::hasEveryOption(m_availableExtensionNames, extensions))
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
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requestedLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = requestedLayers.data();

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
}

std::vector<vko::PhysicalDevice> vko::Instance::findPhysicalDevices()
{
    uint32_t count = 0;
    VKR_ASSERT(vkEnumeratePhysicalDevices(m_handle, &count, nullptr));
    if (count == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> physicalDeviceHandles(count);
    VKR_ASSERT(vkEnumeratePhysicalDevices(m_handle, &count, physicalDeviceHandles.data()));

    std::vector<vko::PhysicalDevice> physicalDevices;
    physicalDevices.reserve(count);
    for (auto const& handle : physicalDeviceHandles)
        physicalDevices.emplace_back(handle);

    return physicalDevices;
}
