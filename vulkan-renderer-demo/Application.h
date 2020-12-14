#pragma once

#include <vector>
#include <string>

struct GLFWwindow;

namespace vkr
{
    class ApplicationImpl;

    class Instance;
    class Surface;
    class PhysicalDevice;
    class Device;

    class Application
    {
    public:
    	Application(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, GLFWwindow* window);

        Instance const& getInstance() const;
        Surface const& getSurface() const;
        PhysicalDevice const& getPhysicalDevice() const;
        Device const& getDevice() const;

    private:
        std::unique_ptr<ApplicationImpl> m_impl;
    };
}
