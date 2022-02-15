#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "UniqueHandle.h"
#include <string>

namespace vkr
{
    class PhysicalDeviceSurfaceContainer;
    class Surface;

    class Instance
    {
    public:
    	Instance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump);
    	~Instance();

        Instance(Instance const&) = default;
        Instance(Instance&&) = default;
        Instance& operator=(Instance const&) = default;
        Instance& operator=(Instance&&) = default;

        VkInstance getHandle() const { return m_handle; }

        std::vector<PhysicalDeviceSurfaceContainer> findPhysicalDevices(Surface const& surface);

    private:
        void createInstance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump);

    private:
    	UniqueHandle<VkInstance> m_handle;

        std::vector<VkLayerProperties> m_availableLayers;
        std::vector<char const*> m_availableLayerNames;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<char const*> m_availableExtensionNames;
    };
}
