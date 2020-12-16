#pragma once

namespace vkr
{
    class Application;

    class Instance;
    class Surface;
    class Device;

    class Object
    {
    public:
        Object() : m_tempApp(nullptr) {}
        Object(Application const& app) : m_tempApp(&app) {}

        Application const& getApp() const { return *m_tempApp; }

        //Object(Application const& app) : m_app(app) {}

        //Application const& getApp() const { return m_app; }

        Instance const& getInstance() const;
        Surface const& getSurface() const;
        Device const& getDevice() const;

    private:
        //Application const& m_app;
        Application const* m_tempApp;
    };
}
