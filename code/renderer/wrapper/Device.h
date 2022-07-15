#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "UniqueHandle.h"

namespace vkr
{
    class PhysicalDeviceSurfaceContainer;
    class Queue;

    class Device
    {
    public:
        explicit Device(PhysicalDeviceSurfaceContainer const& physicalDeviceSurfaceContainer, std::vector<const char*> const& extensions);
    	~Device();

        Device(Device const&) = default;
        Device(Device&&) = default;
        Device& operator=(Device const&) = default;
        Device& operator=(Device&&) = default;

        void waitIdle() const;

        VkDevice getHandle() const { return m_handle; }

		std::vector<Queue> const& getQueues() const { return m_queues; }
        Queue const& getGraphicsQueue() const { return *m_graphicsQueue; }
        Queue const& getPresentQueue() const { return *m_presentQueue; }

    private:
    	UniqueHandle<VkDevice> m_handle;

        std::vector<Queue> m_queues;

        Queue const* m_graphicsQueue = nullptr;
        Queue const* m_presentQueue = nullptr;
    };
}
