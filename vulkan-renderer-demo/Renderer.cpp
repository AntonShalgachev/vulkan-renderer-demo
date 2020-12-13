#include "Renderer.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "CommandPool.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "PhysicalDeviceSurfaceContainer.h"

namespace
{
    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    bool isDeviceSuitable(vkr::PhysicalDeviceSurfaceContainer const& container)
    {
        auto const& physicalDevice = container.getPhysicalDevice();
        auto const& parameters = container.getParameters();

        bool const areExtensionsSupported = physicalDevice.areExtensionsSupported(DEVICE_EXTENSIONS);

        bool swapchainSupported = false;
        if (areExtensionsSupported)
        {
            swapchainSupported = !parameters.getFormats().empty() && !parameters.getPresentModes().empty();
        }

        return parameters.getQueueFamilyIndices().isValid() && areExtensionsSupported && swapchainSupported && physicalDevice.getFeatures().samplerAnisotropy;
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
        , m_physicalDevices(m_instance.findPhysicalDevices(m_surface))
    {
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Renderer::~Renderer() = default;

    void Renderer::onSurfaceChanged()
    {
        m_surface.onSurfaceChanged();

        getPhysicalDeviceSurfaceContainer().getParameters().onSurfaceChanged();
    }

    VkPhysicalDevice Renderer::getPhysicalDevice() const
    {
        return getPhysicalDeviceSurfaceContainer().getPhysicalDevice().getHandle();
    }

    VkDevice Renderer::getDevice() const
    {
        return m_device->getHandle();
    }

    VkCommandPool Renderer::getCommandPool() const
    {
        return m_commandPool->getHandle();
    }

    vkr::PhysicalDeviceSurfaceParameters const& Renderer::getPhysicalDeviceSurfaceParameters() const
    {
        return getPhysicalDeviceSurfaceContainer().getParameters();
    }

    vkr::QueueFamilyIndices const& Renderer::getQueueFamilyIndices() const
    {
        return getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices();
    }

    void Renderer::pickPhysicalDevice()
    {
        for (std::size_t index = 0; index < m_physicalDevices.size(); index++)
        {
            auto const& physicalDevice = m_physicalDevices[index];

            if (isDeviceSuitable(physicalDevice))
            {
                m_currentPhysicalDeviceIndex = index;
                break;
            }
        }
         
        if (m_currentPhysicalDeviceIndex >= m_physicalDevices.size())
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    void Renderer::createLogicalDevice()
    {
        auto const& indices = getQueueFamilyIndices();

        m_device = std::make_unique<Device>(getPhysicalDeviceSurfaceContainer().getPhysicalDevice(), DEVICE_EXTENSIONS, indices.getGraphicsIndex());

        vkGetDeviceQueue(m_device->getHandle(), indices.getGraphicsIndex(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device->getHandle(), indices.getPresentIndex(), 0, &m_presentQueue);
    }

    void Renderer::createCommandPool()
    {
        m_commandPool = std::make_unique<CommandPool>(*m_device, getQueueFamilyIndices().getGraphicsIndex());
    }

    vkr::PhysicalDeviceSurfaceContainer const& Renderer::getPhysicalDeviceSurfaceContainer() const
    {
        return m_physicalDevices[m_currentPhysicalDeviceIndex];
    }

    vkr::PhysicalDeviceSurfaceContainer& Renderer::getPhysicalDeviceSurfaceContainer()
    {
        return m_physicalDevices[m_currentPhysicalDeviceIndex];
    }

}
