#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDevice;

    class Instance
    {
    public:
    	Instance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump);
    	~Instance();

        void createInstance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump);
        void findPhysicalDevices();

        VkInstance const& getHandle() const { return m_handle; }
        std::vector<std::shared_ptr<PhysicalDevice>> const& getPhysicalDevices() const { return m_physicalDevices; }

    private:
    	VkInstance m_handle = VK_NULL_HANDLE;

        std::vector<VkLayerProperties> m_availableLayers;
        std::vector<char const*> m_availableLayerNames;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<char const*> m_availableExtensionNames;

        std::vector<std::shared_ptr<PhysicalDevice>> m_physicalDevices;
    };
}
