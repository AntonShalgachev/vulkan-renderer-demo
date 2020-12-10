#pragma once

#include "framework.h"

namespace vkr
{
    class Instance
    {
    public:
    	Instance(std::string const& appName, std::vector<char const*> extensions, bool enableValidation, bool enableApiDump);
    	~Instance();

        VkInstance const& getHandle() const { return m_handle; }

    private:
    	VkInstance m_handle = VK_NULL_HANDLE;

        std::vector<VkLayerProperties> m_availableLayers;
        std::vector<char const*> m_availableLayerNames;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<char const*> m_availableExtensionNames;
    };
}
