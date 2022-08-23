#pragma once

#include <functional>
#include <string>
#include <memory>

#include <vulkan/vulkan.h> // TODO remove

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

    // TODO rename to Context
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
        
        void setDebugName(VkQueue handle, char const* name) const;
        void setDebugName(VkSemaphore handle, char const* name) const;
        void setDebugName(VkInstance handle, char const* name) const;

    private:
        std::unique_ptr<ApplicationImpl> m_impl;
        std::unique_ptr<CommandPool> m_shortLivedCommandPool;
    };
}
