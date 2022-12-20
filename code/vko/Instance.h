#pragma once

#include "vko/UniqueHandle.h"

#include "nstl/vector.h"
#include "nstl/string.h"
#include "nstl/function.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class PhysicalDevice;
    struct DebugMessage;

    class Instance
    {
    public:
    	Instance(char const* appName, nstl::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump, nstl::function<void(DebugMessage)> onDebugMessage);
    	~Instance();

        Instance(Instance const&) = default;
        Instance(Instance&&) = default;
        Instance& operator=(Instance const&) = default;
        Instance& operator=(Instance&&) = default;

        VkInstance getHandle() const { return m_handle; }

        nstl::vector<vko::PhysicalDevice> findPhysicalDevices();

#define SET_DEBUG_NAME_FUNC(T, ObjectType) \
    void setDebugName(VkDevice device, T handle, char const* name) const { return setDebugName(device, reinterpret_cast<uint64_t>(handle), ObjectType, name); } \
    void setDebugName(VkDevice device, T handle, nstl::string const& name) const { return setDebugName(device, reinterpret_cast<uint64_t>(handle), ObjectType, name.c_str()); }

        SET_DEBUG_NAME_FUNC(VkDevice, VK_OBJECT_TYPE_DEVICE);
        SET_DEBUG_NAME_FUNC(VkQueue, VK_OBJECT_TYPE_QUEUE);
        SET_DEBUG_NAME_FUNC(VkRenderPass, VK_OBJECT_TYPE_RENDER_PASS);
        SET_DEBUG_NAME_FUNC(VkSemaphore, VK_OBJECT_TYPE_SEMAPHORE);
        SET_DEBUG_NAME_FUNC(VkFence, VK_OBJECT_TYPE_FENCE);
        SET_DEBUG_NAME_FUNC(VkCommandPool, VK_OBJECT_TYPE_COMMAND_POOL);
        SET_DEBUG_NAME_FUNC(VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER);
        SET_DEBUG_NAME_FUNC(VkSwapchainKHR, VK_OBJECT_TYPE_SWAPCHAIN_KHR);
        SET_DEBUG_NAME_FUNC(VkImage, VK_OBJECT_TYPE_IMAGE);
        SET_DEBUG_NAME_FUNC(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW);
        SET_DEBUG_NAME_FUNC(VkFramebuffer, VK_OBJECT_TYPE_FRAMEBUFFER);

#undef SET_DEBUG_NAME_FUNC

        void dispatchDebugMessage(DebugMessage message);

    private:
        void createInstance(char const* appName, nstl::vector<char const*> const& extensions, bool enableValidation, bool enableApiDump);
        void findFunctions(bool enableValidation);
        void createDebugMessenger();

        void setDebugName(VkDevice device, uint64_t handle, VkObjectType type, char const* name) const;

    private:
    	UniqueHandle<VkInstance> m_handle;

        UniqueHandle<VkDebugUtilsMessengerEXT> m_debugMessenger;
        nstl::function<void(DebugMessage)> m_onDebugMessage;

        nstl::vector<VkLayerProperties> m_availableLayers;
        nstl::vector<char const*> m_availableLayerNames;

        nstl::vector<VkExtensionProperties> m_availableExtensions;
        nstl::vector<char const*> m_availableExtensionNames;

        PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT = nullptr;
        PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT = nullptr;
    };
}
