#pragma once

#include <vector>
#include <string>

namespace vkr
{
    class ApplicationImpl;

    class Instance;
    class Surface;
    class PhysicalDevice;
    class Device;
    class Window;
    class PhysicalDeviceSurfaceParameters;
    class CommandPool;

    class Application
    {
    public:
    	Application(std::string const& name, bool enableValidation, bool enableApiDump, Window const& window);
        ~Application();

        Instance const& getInstance() const;
        Surface const& getSurface() const;
        Device const& getDevice() const;
        CommandPool const& getCommandPool() const;

        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;
        PhysicalDevice const& getPhysicalDevice() const;

        void onSurfaceChanged();

    private:
        std::unique_ptr<ApplicationImpl> m_impl;
    };
}
