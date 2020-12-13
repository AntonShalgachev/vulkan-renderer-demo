#include "Renderer.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "CommandPool.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"

namespace
{
    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    bool isDeviceSuitable(vkr::PhysicalDevice const& physicalDevice, vkr::PhysicalDeviceSurfaceParameters const& physicalDeviceSurfaceParameters, vkr::QueueFamilyIndices const& indices)
    {
        bool const areExtensionsSupported = physicalDevice.areExtensionsSupported(DEVICE_EXTENSIONS);

        bool swapchainSupported = false;
        if (areExtensionsSupported)
        {
            swapchainSupported = !physicalDeviceSurfaceParameters.getFormats().empty() && !physicalDeviceSurfaceParameters.getPresentModes().empty();
        }

        return indices.isValid() && areExtensionsSupported && swapchainSupported && physicalDevice.getFeatures().samplerAnisotropy;
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
        , m_physicalDevices(m_instance.findPhysicalDevices())
    {
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Renderer::~Renderer() = default;

    void Renderer::onSurfaceChanged()
    {
        m_surface.onSurfaceChanged();

        m_physicalDeviceSurfaceParameters->onSurfaceChanged();
    }

    VkPhysicalDevice Renderer::getPhysicalDevice() const
    {
        return m_physicalDevices[m_currentPhysicalDeviceIndex].getHandle();
    }

    VkDevice Renderer::getDevice() const
    {
        return m_device->getHandle();
    }

    VkCommandPool Renderer::getCommandPool() const
    {
        return m_commandPool->getHandle();
    }

    void Renderer::pickPhysicalDevice()
    {
        for (std::size_t index = 0; index < m_physicalDevices.size(); index++)
        {
            auto const& physicalDevice = m_physicalDevices[index];
            auto parameters = std::make_unique<PhysicalDeviceSurfaceParameters>(physicalDevice, m_surface);
            auto indices = std::make_unique<QueueFamilyIndices>(physicalDevice, *parameters);

            if (isDeviceSuitable(physicalDevice, *parameters, *indices))
            {
                m_currentPhysicalDeviceIndex = index;
                m_physicalDeviceSurfaceParameters = std::move(parameters);
                m_queueFamilyIndices = std::move(indices);
                break;
            }
        }
         
        if (m_currentPhysicalDeviceIndex >= m_physicalDevices.size())
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    void Renderer::createLogicalDevice()
    {
        m_device = std::make_unique<Device>(m_physicalDevices[m_currentPhysicalDeviceIndex], DEVICE_EXTENSIONS, m_queueFamilyIndices->getGraphicsIndex());

        vkGetDeviceQueue(m_device->getHandle(), m_queueFamilyIndices->getGraphicsIndex(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device->getHandle(), m_queueFamilyIndices->getPresentIndex(), 0, &m_presentQueue);
    }

    void Renderer::createCommandPool()
    {
        m_commandPool = std::make_unique<CommandPool>(*m_device, m_queueFamilyIndices->getGraphicsIndex());
    }
}
