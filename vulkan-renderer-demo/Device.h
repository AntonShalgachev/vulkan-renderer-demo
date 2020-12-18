#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDeviceSurfaceContainer;
    class Queue;

    class Device
    {
    public:
        explicit Device(PhysicalDeviceSurfaceContainer const& physicalDeviceSurfaceContainer, std::vector<const char*> const& extensions);
    	~Device();

        Device(Device const&) = delete;
        Device(Device&&) = delete;
        Device& operator=(Device const&) = delete;
        Device& operator=(Device&&) = delete;

        VkDevice const& getHandle() const { return m_handle; }

        Queue const& getGraphicsQueue() const { return *m_graphicsQueue; }
        Queue const& getPresentQueue() const { return *m_presentQueue; }

    private:
    	VkDevice m_handle = VK_NULL_HANDLE;

        std::vector<Queue> m_queues;

        Queue const* m_graphicsQueue = nullptr;
        Queue const* m_presentQueue = nullptr;
    };
}
