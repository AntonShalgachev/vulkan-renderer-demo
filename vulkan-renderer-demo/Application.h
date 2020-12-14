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

    class Application
    {
    public:
    	Application(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window);

        Instance const& getInstance() const;
        Surface const& getSurface() const;
        PhysicalDevice const& getPhysicalDevice() const;
        Device const& getDevice() const;

    private:
        std::unique_ptr<ApplicationImpl> m_impl;
    };
}
