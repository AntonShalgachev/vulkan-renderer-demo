#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace vkr
{
    class PhysicalDeviceSurfaceContainer;
    class Surface;

    class Instance
    {
    public:
    	Instance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump);
    	~Instance();

        Instance(Instance const&) = delete;
        Instance(Instance&&) = delete;
        Instance& operator=(Instance const&) = delete;
        Instance& operator=(Instance&&) = delete;

        VkInstance const& getHandle() const { return m_handle; }

        std::vector<PhysicalDeviceSurfaceContainer> findPhysicalDevices(Surface const& surface);

    private:
        void createInstance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump);

    private:
    	VkInstance m_handle = VK_NULL_HANDLE;

        std::vector<VkLayerProperties> m_availableLayers;
        std::vector<char const*> m_availableLayerNames;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<char const*> m_availableExtensionNames;
    };
}
