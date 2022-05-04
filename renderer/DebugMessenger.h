#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

#include <functional>

namespace vkr
{
    class Instance;
    struct DebugMessage;

    class DebugMessenger
    {
    public:
        DebugMessenger(Instance const& instance, std::function<void(DebugMessage)> onDebugMessage);
        ~DebugMessenger();

        VkDebugUtilsMessengerEXT getHandle() const { return m_handle; }

        void onMessage(DebugMessage message);

    private:
        UniqueHandle<VkDebugUtilsMessengerEXT> m_handle;

        std::function<void(DebugMessage)> m_onDebugMessage;
        Instance const& m_instance;

        PFN_vkCreateDebugUtilsMessengerEXT m_createFunc = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT m_destroyFunc = nullptr;
    };
}
