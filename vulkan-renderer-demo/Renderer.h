#pragma once

#include "framework.h"
#include "Instance.h"
#include "Surface.h"
#include "Device.h"

namespace vkr
{
    class PhysicalDevice;
    class CommandPool;
    class PhysicalDeviceSurfaceContainer;
    class PhysicalDeviceSurfaceParameters;
    class QueueFamilyIndices;
    class Window;

    class Renderer
    {
    public:
        Renderer(Window const& window);
        ~Renderer();

        void onSurfaceChanged();

        VkSurfaceKHR getSurfaceHandle() const { return m_surface.getHandle(); }
        VkDevice getDevice() const;
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkCommandPool getCommandPool() const;

        PhysicalDevice const& getPhysicalDevice() const;
        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;
        QueueFamilyIndices const& getQueueFamilyIndices() const;

        int getWidth() const { return m_surface.getWidth(); }
        int getHeight() const { return m_surface.getHeight(); }

    private:
        void getDeviceQueues();
        void createCommandPool();

        PhysicalDeviceSurfaceContainer const& getPhysicalDeviceSurfaceContainer() const;
        PhysicalDeviceSurfaceContainer& getPhysicalDeviceSurfaceContainer();

        Instance m_instance;
        Surface m_surface;
        std::vector<vkr::PhysicalDeviceSurfaceContainer> m_physicalDevices;
        std::size_t m_currentPhysicalDeviceIndex = std::numeric_limits<std::size_t>::max();
        Device m_device;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        std::unique_ptr<CommandPool> m_commandPool;
    };
}
