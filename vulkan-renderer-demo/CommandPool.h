#pragma once

#include "framework.h"

namespace vkr
{
    class Device;

    class CommandPool
    {
    public:
    	explicit CommandPool(Device const& device, uint32_t queueFamilyIndex);
    	~CommandPool();

        CommandPool(CommandPool const&) = delete;
        CommandPool(CommandPool&&) = delete;
        CommandPool& operator=(CommandPool const&) = delete;
        CommandPool& operator=(CommandPool&&) = delete;

        VkCommandPool const& getHandle() const { return m_handle; }

    private:
    	VkCommandPool m_handle = VK_NULL_HANDLE;

        Device const& m_device;
    };
}
