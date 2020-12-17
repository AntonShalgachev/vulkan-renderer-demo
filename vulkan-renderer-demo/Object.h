#pragma once

namespace vkr
{
    class Application;

    class Instance;
    class Surface;
    class Device;
    class PhysicalDevice;
    class PhysicalDeviceSurfaceParameters;

    class Object
    {
    public:
        Object() : m_tempApp(nullptr) {}
        Object(Application const& app) : m_tempApp(&app) {}

        Application const& getApp() const { return *m_tempApp; }

        //Object(Application const& app) : m_app(app) {}

        //Application const& getApp() const { return m_app; }

        Object(Object const&) = delete;
        Object(Object&&) = default;
        Object& operator=(Object const&) = delete;
        Object& operator=(Object&&) = delete;

        Instance const& getInstance() const;
        Surface const& getSurface() const;
        Device const& getDevice() const;
        PhysicalDevice const& getPhysicalDevice() const;
        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;

    private:
        //Application const& m_app;
        Application const* m_tempApp;
    };
}
