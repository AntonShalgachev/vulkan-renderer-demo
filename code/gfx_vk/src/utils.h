#pragma once

#include "common/fmt.h"

#include <vulkan/vulkan.h>

#include <assert.h>

#ifdef NDEBUG
#define GFX_VK_VERIFY(cmd) (cmd)
#else
#define GFX_VK_VERIFY(cmd) gfx_vk::do_assert((cmd), #cmd)
#endif

namespace gfx_vk
{
    template<typename T>
    class unique_handle
    {
    public:
        unique_handle() = default;
        explicit unique_handle(nullptr_t) : unique_handle() {}
        unique_handle(T const& handle) : m_handle(handle) {}

        unique_handle(unique_handle const&) = delete;
        unique_handle(unique_handle&& rhs) : m_handle(rhs.m_handle)
        {
            rhs.m_handle = VK_NULL_HANDLE;
        }
        unique_handle& operator=(unique_handle const&) = delete;
        unique_handle& operator=(unique_handle&& rhs)
        {
            if (this != &rhs)
            {
                m_handle = rhs.m_handle;
                rhs.m_handle = VK_NULL_HANDLE;
            }

            return *this;
        }

        unique_handle& operator=(nullptr_t)
        {
            m_handle = VK_NULL_HANDLE;
            return *this;
        }

        T const& get() const { return m_handle; }
        T& get() { return m_handle; }

        explicit operator bool() const
        {
            return m_handle != VK_NULL_HANDLE;
        }

        operator T() const
        {
            return m_handle;
        }

        ~unique_handle() = default;

    private:
        T m_handle = VK_NULL_HANDLE;
    };

    inline void do_assert(VkResult result, char const*)
    {
        assert(result == VK_SUCCESS);
    }

    template<typename T> constexpr inline VkObjectType object_type_v = VK_OBJECT_TYPE_UNKNOWN;

    template<> constexpr inline VkObjectType object_type_v<VkInstance> = VK_OBJECT_TYPE_INSTANCE;
    template<> constexpr inline VkObjectType object_type_v<VkPhysicalDevice> = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
    template<> constexpr inline VkObjectType object_type_v<VkDevice> = VK_OBJECT_TYPE_DEVICE;
    template<> constexpr inline VkObjectType object_type_v<VkQueue> = VK_OBJECT_TYPE_QUEUE;
    template<> constexpr inline VkObjectType object_type_v<VkCommandBuffer> = VK_OBJECT_TYPE_COMMAND_BUFFER;
    template<> constexpr inline VkObjectType object_type_v<VkDeviceMemory> = VK_OBJECT_TYPE_DEVICE_MEMORY;
    template<> constexpr inline VkObjectType object_type_v<VkCommandPool> = VK_OBJECT_TYPE_COMMAND_POOL;
    template<> constexpr inline VkObjectType object_type_v<VkBuffer> = VK_OBJECT_TYPE_BUFFER;
    template<> constexpr inline VkObjectType object_type_v<VkBufferView> = VK_OBJECT_TYPE_BUFFER_VIEW;
    template<> constexpr inline VkObjectType object_type_v<VkImage> = VK_OBJECT_TYPE_IMAGE;
    template<> constexpr inline VkObjectType object_type_v<VkImageView> = VK_OBJECT_TYPE_IMAGE_VIEW;
    template<> constexpr inline VkObjectType object_type_v<VkShaderModule> = VK_OBJECT_TYPE_SHADER_MODULE;
    template<> constexpr inline VkObjectType object_type_v<VkPipeline> = VK_OBJECT_TYPE_PIPELINE;
    template<> constexpr inline VkObjectType object_type_v<VkPipelineLayout> = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    template<> constexpr inline VkObjectType object_type_v<VkSampler> = VK_OBJECT_TYPE_SAMPLER;
    template<> constexpr inline VkObjectType object_type_v<VkDescriptorSet> = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    template<> constexpr inline VkObjectType object_type_v<VkDescriptorSetLayout> = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
    template<> constexpr inline VkObjectType object_type_v<VkDescriptorPool> = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
    template<> constexpr inline VkObjectType object_type_v<VkFence> = VK_OBJECT_TYPE_FENCE;
    template<> constexpr inline VkObjectType object_type_v<VkSemaphore> = VK_OBJECT_TYPE_SEMAPHORE;
    template<> constexpr inline VkObjectType object_type_v<VkEvent> = VK_OBJECT_TYPE_EVENT;
    template<> constexpr inline VkObjectType object_type_v<VkQueryPool> = VK_OBJECT_TYPE_QUERY_POOL;
    template<> constexpr inline VkObjectType object_type_v<VkFramebuffer> = VK_OBJECT_TYPE_FRAMEBUFFER;
    template<> constexpr inline VkObjectType object_type_v<VkRenderPass> = VK_OBJECT_TYPE_RENDER_PASS;
    template<> constexpr inline VkObjectType object_type_v<VkPipelineCache> = VK_OBJECT_TYPE_PIPELINE_CACHE;
    template<> constexpr inline VkObjectType object_type_v<VkDescriptorUpdateTemplate> = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
    template<> constexpr inline VkObjectType object_type_v<VkSamplerYcbcrConversion> = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
    template<> constexpr inline VkObjectType object_type_v<VkPrivateDataSlot> = VK_OBJECT_TYPE_PRIVATE_DATA_SLOT;
    template<> constexpr inline VkObjectType object_type_v<VkSurfaceKHR> = VK_OBJECT_TYPE_SURFACE_KHR;
    template<> constexpr inline VkObjectType object_type_v<VkSwapchainKHR> = VK_OBJECT_TYPE_SWAPCHAIN_KHR;
    template<> constexpr inline VkObjectType object_type_v<VkDebugReportCallbackEXT> = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;

    template<typename T>
    concept vulkan_handle = object_type_v<T> != VK_OBJECT_TYPE_UNKNOWN;
}
