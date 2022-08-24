#pragma once

#include <functional>
#include <string>
#include <memory>

#include <vulkan/vulkan.h> // TODO remove

namespace vko
{
    class Instance;
    class Surface;
    class PhysicalDevice;
    class Device;
    class CommandPool;
    struct DebugMessage;
}

namespace vkr
{
    class GlfwWindow;
    class ApplicationImpl;
    class PhysicalDeviceSurfaceParameters;

    // TODO rename to Context
    class Application
    {
    public:
        Application(std::string const& name, bool enableValidation, bool enableApiDump, GlfwWindow const& window, std::function<void(vko::DebugMessage)> onDebugMessage = {});
        ~Application();

        vko::Instance const& getInstance() const;
        vko::Surface const& getSurface() const;
        vko::Device const& getDevice() const;

        vko::CommandPool const& getShortLivedCommandPool() const;

        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const;
        vko::PhysicalDevice const& getPhysicalDevice() const;

        void onSurfaceChanged();
        
        void setDebugName(VkQueue handle, char const* name) const;
        void setDebugName(VkSemaphore handle, char const* name) const;
        void setDebugName(VkInstance handle, char const* name) const;

    private:
        std::unique_ptr<ApplicationImpl> m_impl;
        std::unique_ptr<vko::CommandPool> m_shortLivedCommandPool;
    };
}
