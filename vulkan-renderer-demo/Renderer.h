#pragma once

#include "framework.h"
#include "Instance.h"
#include "Surface.h"

namespace vkr
{
    class PhysicalDevice;
    class Device;
    class CommandPool;
    class PhysicalDeviceSurfaceParameters;
    class QueueFamilyIndices;

    class Renderer
    {
    public:
        Renderer(GLFWwindow* window);
        ~Renderer();

        void onSurfaceChanged();

        VkSurfaceKHR getSurfaceHandle() const { return m_surface.getHandle(); }
        VkPhysicalDevice getPhysicalDevice() const;
        VkDevice getDevice() const;
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkCommandPool getCommandPool() const;

        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const { return *m_physicalDeviceSurfaceParameters; }
        QueueFamilyIndices const& getQueueFamilyIndices() const;

        int getWidth() const { return m_surface.getWidth(); }
        int getHeight() const { return m_surface.getHeight(); }

        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        Instance m_instance;
        Surface m_surface;
        std::vector<vkr::PhysicalDevice> m_physicalDevices;
        std::size_t m_currentPhysicalDeviceIndex = std::numeric_limits<std::size_t>::max();
        std::unique_ptr<Device> m_device;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        std::unique_ptr<CommandPool> m_commandPool;

        std::unique_ptr<PhysicalDeviceSurfaceParameters> m_physicalDeviceSurfaceParameters;
    };
}
