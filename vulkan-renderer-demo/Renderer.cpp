#include "Renderer.h"

namespace
{
    const std::vector<const char*> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation",
    };

    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

#ifdef NDEBUG
    const bool ENABLE_VALIDATION_LAYERS = false;
#else
    const bool ENABLE_VALIDATION_LAYERS = true;
#endif

    bool hasRequiredValidationLayer(const std::vector<std::string>& requiredValidationLayerNames)
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::cout << "Available validation layers" << std::endl;
        for (const auto& layer : availableLayers)
            std::cout << '\t' << layer.layerName << " v." << layer.specVersion << ": " << layer.description << std::endl;

        std::vector<std::string> availableLayerNames;
        availableLayerNames.reserve(layerCount);
        for (const auto& layer : availableLayers)
            availableLayerNames.push_back(layer.layerName);

        std::cout << "Required validation layers" << std::endl;
        for (const auto& name : requiredValidationLayerNames)
            std::cout << '\t' << name << std::endl;

        for (const auto& name : requiredValidationLayerNames)
        {
            auto it = std::find(availableLayerNames.begin(), availableLayerNames.end(), name);
            if (it == availableLayerNames.end())
                return false;
        }

        return false;
    }

    bool hasRequiredExtensions(const std::vector<std::string>& requiredExtensionNames)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

        std::vector<std::string> supportedExtensionNames;
        supportedExtensionNames.reserve(extensionCount);
        for (const auto& supportedExtension : supportedExtensions)
            supportedExtensionNames.push_back(supportedExtension.extensionName);

        std::cout << "Available extensions" << std::endl;
        for (const auto& name : supportedExtensionNames)
            std::cout << '\t' << name << std::endl;

        std::cout << "Required extensions" << std::endl;
        for (const auto& name : requiredExtensionNames)
            std::cout << '\t' << name << std::endl;

        for (const auto& requiredExtensionName : requiredExtensionNames)
        {
            auto it = std::find(supportedExtensionNames.begin(), supportedExtensionNames.end(), requiredExtensionName);
            if (it == supportedExtensionNames.end())
                return false;
        }

        return true;
    }

    bool doesDeviceSupportExtensions(VkPhysicalDevice device, const std::vector<const char*>& extensions)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool isDeviceSuitable(VkPhysicalDevice device, vkr::Renderer::PhysicalDeviceProperties properties)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        const bool areExtensionsSupported = doesDeviceSupportExtensions(device, DEVICE_EXTENSIONS);

        bool swapchainSupported = false;
        if (areExtensionsSupported)
        {
            swapchainSupported = !properties.swapchainSupportDetails.formats.empty() && !properties.swapchainSupportDetails.presentModes.empty();
        }

        return properties.queueFamilyIndices.IsComplete() && areExtensionsSupported && swapchainSupported && deviceFeatures.samplerAnisotropy;
    }

    vkr::Renderer::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        vkr::Renderer::QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
                indices.presentFamily = i;

            if (indices.IsComplete())
                break;

            i++;
        }

        return indices;
    }

    vkr::Renderer::SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        vkr::Renderer::SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        {
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if (formatCount > 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }
        }

        {
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            if (presentModeCount > 0)
            {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }
        }

        return details;
    }

    vkr::Renderer::PhysicalDeviceProperties calculatePhysicalDeviceProperties(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        vkr::Renderer::PhysicalDeviceProperties properties;
        properties.queueFamilyIndices = findQueueFamilies(device, surface);
        properties.swapchainSupportDetails = querySwapchainSupport(device, surface);

        return properties;
    }
}

namespace vkr
{
    Renderer::Renderer(GLFWwindow* window)
    {
        glfwGetFramebufferSize(window, &m_width, &m_height);

        createVulkanInstance();
        createSurface(window);
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Renderer::~Renderer()
    {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    void Renderer::OnSurfaceChanged(int width, int height)
    {
        m_width = width;
        m_height = height;
        m_physicalDeviceProperties = calculatePhysicalDeviceProperties(m_physicalDevice, m_surface);
    }

    void Renderer::createVulkanInstance()
    {
        std::vector<std::string> requiredValidationLayerNames;
        for (auto name : VALIDATION_LAYERS)
            requiredValidationLayerNames.push_back(name);
        if (ENABLE_VALIDATION_LAYERS && hasRequiredValidationLayer(requiredValidationLayerNames))
            throw std::runtime_error("Some of the required validation layers aren't supported");

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<std::string> requiredExtensionNames;
        requiredExtensionNames.reserve(glfwExtensionCount);
        for (uint32_t i = 0; i < glfwExtensionCount; i++)
            requiredExtensionNames.push_back(glfwExtensions[i]);

        if (!hasRequiredExtensions(requiredExtensionNames))
            throw std::runtime_error("Some of the required extensions aren't supported");

        instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
        instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

        if (ENABLE_VALIDATION_LAYERS)
        {
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            instanceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        }
        else
        {
            instanceCreateInfo.enabledLayerCount = 0;
        }

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create Vulkan instance");
    }

    void Renderer::createSurface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
    }

    void Renderer::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            PhysicalDeviceProperties properties = calculatePhysicalDeviceProperties(device, m_surface);

            if (isDeviceSuitable(device, properties))
            {
                m_physicalDevice = device;
                m_physicalDeviceProperties = properties;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    void Renderer::createLogicalDevice()
    {
        QueueFamilyIndices indices = m_physicalDeviceProperties.queueFamilyIndices;

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
        deviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

        if (ENABLE_VALIDATION_LAYERS)
        {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
            deviceCreateInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
            throw std::runtime_error("failed to create logical device!");

        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    void Renderer::createCommandPool()
    {
        vkr::Renderer::QueueFamilyIndices queueFamilyIndices = getPhysicalDeviceProperties().queueFamilyIndices;

        VkCommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolCreateInfo.flags = 0;

        if (vkCreateCommandPool(m_device, &poolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool!");
    }
}
