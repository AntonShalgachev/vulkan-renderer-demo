#pragma once

#include "framework.h"

namespace vkr
{
    class PhysicalDevice;

    class Device
    {
    public:
    	explicit Device(PhysicalDevice const& physicalDevice, std::vector<const char*> const& extensions, uint32_t queueFamilyIndex);
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
