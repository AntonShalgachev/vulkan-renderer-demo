#include "Renderer.h"
#include "PhysicalDevice.h"
#include "Device.h"

namespace
{
    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    bool isDeviceSuitable(vkr::PhysicalDevice const& physicalDevice, vkr::Renderer::PhysicalDeviceProperties properties)
    {
        auto const& features = physicalDevice.getFeatures();

        bool const areExtensionsSupported = physicalDevice.areExtensionsSupported(DEVICE_EXTENSIONS);

        bool swapchainSupported = false;
        if (areExtensionsSupported)
        {
            swapchainSupported = !properties.swapchainSupportDetails.formats.empty() && !properties.swapchainSupportDetails.presentModes.empty();
        }

        return properties.queueFamilyIndices.IsComplete() && areExtensionsSupported && swapchainSupported && features.samplerAnisotropy;
    }

    vkr::Renderer::QueueFamilyIndices findQueueFamilies(vkr::PhysicalDevice const& physicalDevice, VkSurfaceKHR surface)
    {
        vkr::Renderer::QueueFamilyIndices indices;

        std::vector<VkQueueFamilyProperties> const& queueFamilies = physicalDevice.getQueueFamilyProperties();

        for (auto i = 0; i < queueFamilies.size(); i++)
        {
            auto const& queueFamily = queueFamilies[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.getHandle(), i, surface, &presentSupport);

            if (presentSupport)
                indices.presentFamily = i;

            if (indices.IsComplete())
                break;
        }

        return indices;
    }

    vkr::Renderer::SwapchainSupportDetails querySwapchainSupport(vkr::PhysicalDevice const& physicalDevice, VkSurfaceKHR surface)
    {
        vkr::Renderer::SwapchainSupportDetails details;

        VkPhysicalDevice handle = physicalDevice.getHandle();

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, &details.capabilities);

        {
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &formatCount, nullptr);

            if (formatCount > 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &formatCount, details.formats.data());
            }
        }

        {
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &presentModeCount, nullptr);

            if (presentModeCount > 0)
            {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &presentModeCount, details.presentModes.data());
            }
        }

        return details;
    }

    vkr::Renderer::PhysicalDeviceProperties calculatePhysicalDeviceProperties(vkr::PhysicalDevice const& physicalDevice, VkSurfaceKHR surface)
    {
        vkr::Renderer::PhysicalDeviceProperties properties;
        properties.queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
        properties.swapchainSupportDetails = querySwapchainSupport(physicalDevice, surface);

        return properties;
    }

    std::vector<char const*> getGlfwExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        return std::vector<char const*>{ glfwExtensions, glfwExtensions + glfwExtensionCount };
    }

    bool const VALIDATION_ENABLED = true;
    bool const API_DUMP_ENABLED = false;
}

namespace vkr
{
    Renderer::Renderer(GLFWwindow* window)
        : m_instance("Vulkan demo", getGlfwExtensions(), VALIDATION_ENABLED, API_DUMP_ENABLED)
        , m_surface(m_instance, window)
    {
        glfwGetFramebufferSize(window, &m_width, &m_height);

        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Renderer::~Renderer()
    {
        vkDestroyCommandPool(m_device->getHandle(), m_commandPool, nullptr);
    }

    void Renderer::OnSurfaceChanged(int width, int height)
    {
        m_width = width;
        m_height = height;

        m_physicalDeviceProperties = calculatePhysicalDeviceProperties(*m_physicalDevice, m_surface.getHandle());
    }

    VkPhysicalDevice Renderer::getPhysicalDevice() const
    {
        return m_physicalDevice->getHandle();
    }

    VkDevice Renderer::getDevice() const
    {
        return m_device->getHandle();
    }

    void Renderer::pickPhysicalDevice()
    {
        auto const& physicalDevices = m_instance.getPhysicalDevices();

        for (const auto& physicalDevice : physicalDevices)
        {
            PhysicalDeviceProperties properties = calculatePhysicalDeviceProperties(*physicalDevice, m_surface.getHandle());

            if (isDeviceSuitable(*physicalDevice, properties))
            {
                m_physicalDevice = physicalDevice;
                m_physicalDeviceProperties = properties;
                break;
            }
        }
         
        if (!m_physicalDevice)
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    void Renderer::createLogicalDevice()
    {
        QueueFamilyIndices const& indices = m_physicalDeviceProperties.queueFamilyIndices;

        m_device = std::make_unique<Device>(*m_physicalDevice, DEVICE_EXTENSIONS, indices.graphicsFamily.value());

        vkGetDeviceQueue(m_device->getHandle(), indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device->getHandle(), indices.presentFamily.value(), 0, &m_presentQueue);
    }

    void Renderer::createCommandPool()
    {
        vkr::Renderer::QueueFamilyIndices queueFamilyIndices = getPhysicalDeviceProperties().queueFamilyIndices;

        VkCommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolCreateInfo.flags = 0;

        if (vkCreateCommandPool(m_device->getHandle(), &poolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool!");
    }
}
