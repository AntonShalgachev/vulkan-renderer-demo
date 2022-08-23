#pragma once

namespace vko
{
    class Instance;
    class Surface;
    class Device;
    class PhysicalDevice;
}

namespace vkr
{
    class Application;
    class PhysicalDeviceSurfaceParameters;

    class Object
    {
    public:
        Object(Application const& app) : m_app(app) {}

        Application const& getApp() const { return m_app; }

        vko::Instance const& getInstance() const;
        vko::Surface const& getSurface() const;
        vko::Device const& getDevice() const;
        vko::PhysicalDevice const& getPhysicalDevice() const;
        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;

    private:
        Application const& m_app;
    };
}
