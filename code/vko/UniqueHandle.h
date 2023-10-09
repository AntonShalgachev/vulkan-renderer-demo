#pragma once

#include "common/fmt.h"

#include <vulkan/vulkan.h>

namespace vko
{
    // Its main purpose is to make it easier to implement move constructors of vulkan objects
    template<typename T>
    class UniqueHandle
    {
    public:
        UniqueHandle() = default;
        explicit UniqueHandle(nullptr_t) : UniqueHandle() {}
        UniqueHandle(T const& handle) : m_handle(handle) {}

        UniqueHandle(UniqueHandle const&) = delete;
        UniqueHandle(UniqueHandle&& rhs) : m_handle(rhs.m_handle)
        {
            rhs.m_handle = VK_NULL_HANDLE;
        }
        UniqueHandle& operator=(UniqueHandle const&) = delete;
        UniqueHandle& operator=(UniqueHandle&& rhs)
        {
            if (this != &rhs)
            {
                m_handle = rhs.m_handle;
                rhs.m_handle = VK_NULL_HANDLE;
            }

            return *this;
        }

        UniqueHandle& operator=(nullptr_t)
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

        ~UniqueHandle() = default;

    private:
        T m_handle = VK_NULL_HANDLE;
    };
}

template<typename T>
struct picofmt::formatter<vko::UniqueHandle<T>> : public picofmt::formatter<void*>
{
    bool format(vko::UniqueHandle<T> const& value, context& ctx) const
    {
        return picofmt::formatter<void*>::format(value.get(), ctx);
    }
};

template<> struct picofmt::formatter<VkInstance> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkPhysicalDevice> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkDevice> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkQueue> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkCommandBuffer> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkDeviceMemory> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkCommandPool> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkBuffer> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkBufferView> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkImage> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkImageView> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkShaderModule> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkPipeline> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkPipelineLayout> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkSampler> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkDescriptorSet> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkDescriptorSetLayout> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkDescriptorPool> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkFence> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkSemaphore> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkEvent> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkQueryPool> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkFramebuffer> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkRenderPass> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkPipelineCache> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkDescriptorUpdateTemplate> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkSamplerYcbcrConversion> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkPrivateDataSlot> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkSurfaceKHR> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkSwapchainKHR> : public picofmt::formatter<void*> {};
template<> struct picofmt::formatter<VkDebugReportCallbackEXT> : public picofmt::formatter<void*> {};
