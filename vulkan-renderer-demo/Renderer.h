#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDevice;
    class CommandPool;
    class PhysicalDeviceSurfaceContainer;
    class PhysicalDeviceSurfaceParameters;
    class QueueFamilyIndices;
    class Window;
    class Application;

    class Renderer
    {
    public:
        Renderer(Window const& window);
        ~Renderer();

        void onSurfaceChanged();

        VkSurfaceKHR getSurfaceHandle() const;
        VkDevice getDevice() const;
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkCommandPool getCommandPool() const;

        PhysicalDevice const& getPhysicalDevice() const;
        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;
        QueueFamilyIndices const& getQueueFamilyIndices() const;

        int getWidth() const;
        int getHeight() const;

    private:
        void getDeviceQueues();
        void createCommandPool();

        std::unique_ptr<Application> m_application;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        std::unique_ptr<CommandPool> m_commandPool;
    };
}
