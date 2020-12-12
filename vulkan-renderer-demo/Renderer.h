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

        void OnSurfaceChanged(int width, int height);

        VkSurfaceKHR getSurfaceHandle() const { return m_surface.getHandle(); }
        VkPhysicalDevice getPhysicalDevice() const;
        VkDevice getDevice() const;
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkCommandPool getCommandPool() const;

        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() { return *m_physicalDeviceSurfaceParameters; }
        QueueFamilyIndices const& getQueueFamilyIndices() { return *m_queueFamilyIndices; }

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        Instance m_instance;
        Surface m_surface;
        std::shared_ptr<PhysicalDevice> m_physicalDevice;
        std::unique_ptr<Device> m_device;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        std::unique_ptr<CommandPool> m_commandPool;

        std::unique_ptr<PhysicalDeviceSurfaceParameters> m_physicalDeviceSurfaceParameters;
        std::unique_ptr<QueueFamilyIndices> m_queueFamilyIndices;

        int m_width = -1;
        int m_height = -1;
    };
}
