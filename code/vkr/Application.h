#pragma once

#include "nstl/function.h"
#include "nstl/unique_ptr.h"

namespace vko
{
    class Instance;
    class Surface;
    class PhysicalDevice;
    class Device;
    struct DebugMessage;
    class Window;
    struct PhysicalDeviceSurfaceParameters;
}

namespace vkr
{
    class ApplicationImpl;

    // TODO rename to Context
    class Application
    {
    public:
        Application(char const* name, bool enableValidation, bool enableApiDump, vko::Window const& window, nstl::function<void(vko::DebugMessage)> onDebugMessage = {});
        ~Application();

        vko::Instance const& getInstance() const;
        vko::Surface const& getSurface() const;
        vko::Device const& getDevice() const;

        vko::PhysicalDevice const& getPhysicalDevice() const;
        vko::PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;

        void onSurfaceChanged();

    private:
        nstl::unique_ptr<ApplicationImpl> m_impl;
    };
}
