#pragma once

#include "UniqueHandle.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Queue;
    class PhysicalDevice;
    class QueueFamily;

    class Device
    {
    public:
        // TODO use std::span for extensions
        explicit Device(vko::PhysicalDevice const& physicalDevice, vko::QueueFamily const& graphics, vko::QueueFamily const& present, nstl::vector<const char*> const& extensions);
    	~Device();

        Device(Device const&) = default;
        Device(Device&&) = default;
        Device& operator=(Device const&) = default;
        Device& operator=(Device&&) = default;

        void waitIdle() const;

        VkDevice getHandle() const { return m_handle; }

		nstl::vector<Queue> const& getQueues() const { return m_queues; }
        Queue const& getGraphicsQueue() const { return *m_graphicsQueue; }
        Queue const& getPresentQueue() const { return *m_presentQueue; }

    private:
    	UniqueHandle<VkDevice> m_handle;

        nstl::vector<Queue> m_queues;

        Queue const* m_graphicsQueue = nullptr;
        Queue const* m_presentQueue = nullptr;
    };
}
