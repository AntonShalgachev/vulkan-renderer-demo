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

        CommandPool const& getCommandPool() const { return *m_commandPool; }

        Application const& getApplication() const { return *m_application; }

    private:
        void createCommandPool();

        std::unique_ptr<Application> m_application;

        std::unique_ptr<CommandPool> m_commandPool;
    };
}
