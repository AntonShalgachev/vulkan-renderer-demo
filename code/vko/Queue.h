#pragma once

#include <vulkan/vulkan.h>

namespace vko
{
    class QueueFamily;
    
    class Queue
    {
    public:
    	Queue(VkQueue handle, uint32_t queueFamily);

        VkQueue getHandle() const { return m_handle; }
        uint32_t getFamily() const { return m_family; }

        void waitIdle() const;

    private:
        VkQueue m_handle = VK_NULL_HANDLE;
        uint32_t m_family;
    };
}
