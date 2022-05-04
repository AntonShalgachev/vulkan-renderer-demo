#pragma once

#include <functional>
#include <string>
#include <memory>

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
    struct DebugMessage;

    class Application
    {
    public:
        Application(std::string const& name, bool enableValidation, bool enableApiDump, Window const& window, std::function<void(DebugMessage)> onDebugMessage = {});
        ~Application();

        Instance const& getInstance() const;
        Surface const& getSurface() const;
        Device const& getDevice() const;

        CommandPool const& getShortLivedCommandPool() const;

        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;
        PhysicalDevice const& getPhysicalDevice() const;

        void onSurfaceChanged();

    private:
        std::unique_ptr<ApplicationImpl> m_impl;
        std::unique_ptr<CommandPool> m_shortLivedCommandPool;
    };
}
