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

        VkDevice const& getHandle() const { return m_handle; }

    private:
    	VkDevice m_handle = VK_NULL_HANDLE;
    };
}
