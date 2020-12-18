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

        Application const& getApplication() const { return *m_application; }

    private:
        std::unique_ptr<Application> m_application;
    };
}
