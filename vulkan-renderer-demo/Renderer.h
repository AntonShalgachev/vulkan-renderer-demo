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
        VkCommandPool getCommandPool() const;

        Application const& getApplication() const { return *m_application; }

        PhysicalDevice const& getPhysicalDevice() const;
        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;
        QueueFamilyIndices const& getQueueFamilyIndices() const;

        int getWidth() const;
        int getHeight() const;

    private:
        void createCommandPool();

        std::unique_ptr<Application> m_application;

        std::unique_ptr<CommandPool> m_commandPool;
    };
}
