#include "Renderer.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "CommandPool.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "PhysicalDeviceSurfaceContainer.h"
#include "Window.h"

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

    std::size_t findSuitablePhysicalDeviceIndex(std::vector<vkr::PhysicalDeviceSurfaceContainer> const& physicalDevices)
    {
        for (std::size_t index = 0; index < physicalDevices.size(); index++)
        {
            auto const& physicalDevice = physicalDevices[index];

            if (isDeviceSuitable(physicalDevice))
            {
                return index;
            }
        }

        throw std::runtime_error("failed to find a suitable GPU!");
    }

    bool const VALIDATION_ENABLED = true;
    bool const API_DUMP_ENABLED = false;
}

namespace vkr
{
    Renderer::Renderer(Window const& window)
        : m_instance("Vulkan demo", window.getRequiredInstanceExtensions(), VALIDATION_ENABLED, API_DUMP_ENABLED)
        , m_surface(m_instance, window)
        , m_physicalDevices(m_instance.findPhysicalDevices(m_surface))
        , m_currentPhysicalDeviceIndex(findSuitablePhysicalDeviceIndex(m_physicalDevices))
        , m_device(getPhysicalDevice(), DEVICE_EXTENSIONS, getQueueFamilyIndices().getGraphicsIndex())
    {
        getDeviceQueues();
        createCommandPool();
    }

    Renderer::~Renderer() = default;

    void Renderer::onSurfaceChanged()
    {
        getPhysicalDeviceSurfaceContainer().getParameters().onSurfaceChanged();
    }

    VkDevice Renderer::getDevice() const
    {
        return m_device.getHandle();
    }

    VkCommandPool Renderer::getCommandPool() const
    {
        return m_commandPool->getHandle();
    }

    vkr::PhysicalDevice const& Renderer::getPhysicalDevice() const
    {
        return getPhysicalDeviceSurfaceContainer().getPhysicalDevice();
    }

    vkr::PhysicalDeviceSurfaceParameters const& Renderer::getPhysicalDeviceSurfaceParameters() const
    {
        return getPhysicalDeviceSurfaceContainer().getParameters();
    }

    vkr::QueueFamilyIndices const& Renderer::getQueueFamilyIndices() const
    {
        return getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices();
    }

    void Renderer::getDeviceQueues()
    {
        auto const& indices = getQueueFamilyIndices();

        vkGetDeviceQueue(m_device.getHandle(), indices.getGraphicsIndex(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device.getHandle(), indices.getPresentIndex(), 0, &m_presentQueue);
    }

    void Renderer::createCommandPool()
    {
        m_commandPool = std::make_unique<CommandPool>(m_device, getQueueFamilyIndices().getGraphicsIndex());
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
