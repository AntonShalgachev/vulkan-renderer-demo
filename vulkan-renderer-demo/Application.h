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

    class Application
    {
    public:
    	Application(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window);
        ~Application();

        Instance const& getInstance() const;
        Surface const& getSurface() const;
        Device const& getDevice() const;

        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;
        PhysicalDevice const& getPhysicalDevice() const;

        void onSurfaceChanged();

    private:
        std::unique_ptr<ApplicationImpl> m_impl;
    };
}
