#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDeviceSurfaceContainer;

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

    private:
    	VkDevice m_handle = VK_NULL_HANDLE;
    };
}
