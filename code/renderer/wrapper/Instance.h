#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <functional>

#include "UniqueHandle.h"

namespace vko
{
    class PhysicalDevice;
    struct DebugMessage;

    class Instance
    {
    public:
    	Instance(std::string const& appName, std::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump, std::function<void(DebugMessage)> onDebugMessage);
    	~Instance();

        Instance(Instance const&) = default;
        Instance(Instance&&) = default;
        Instance& operator=(Instance const&) = default;
        Instance& operator=(Instance&&) = default;

        VkInstance getHandle() const { return m_handle; }

        std::vector<vko::PhysicalDevice> findPhysicalDevices();

        void dispatchDebugMessage(DebugMessage message);

    private:
        void createInstance(std::string const& appName, std::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump);
        void findFunctions();
        void createDebugMessenger();

    private:
    	UniqueHandle<VkInstance> m_handle;

        UniqueHandle<VkDebugUtilsMessengerEXT> m_debugMessenger;
        std::function<void(DebugMessage)> m_onDebugMessage;

        std::vector<VkLayerProperties> m_availableLayers;
        std::vector<char const*> m_availableLayerNames;

        std::vector<VkExtensionProperties> m_availableExtensions;
        std::vector<char const*> m_availableExtensionNames;

        PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT = nullptr;
        PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT = nullptr;
    };
}
