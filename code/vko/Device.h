#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include "nstl/vector.h"
#include "nstl/span.h"

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
        explicit Device(vko::PhysicalDevice const& physicalDevice, vko::QueueFamily const& graphics, vko::QueueFamily const& present, nstl::span<const char* const> extensions);
    	~Device();

        Device(Device const&) = default;
        Device(Device&&) = default;
        Device& operator=(Device const&) = default;
        Device& operator=(Device&&) = default;

        void waitIdle() const;

        VkDevice getHandle() const { return m_handle; }

		nstl::vector<Queue> const& getQueues() const { return m_queues; } // TODO use span
        Queue const& getGraphicsQueue() const { return *m_graphicsQueue; }
        Queue const& getPresentQueue() const { return *m_presentQueue; }

    private:
        void createAllocator();

    private:
        Allocator m_allocator{ AllocatorScope::Device };
    	UniqueHandle<VkDevice> m_handle;

        nstl::vector<Queue> m_queues;

        Queue const* m_graphicsQueue = nullptr;
        Queue const* m_presentQueue = nullptr;
    };
}
