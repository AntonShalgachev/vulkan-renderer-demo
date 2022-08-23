#pragma once

#include <vulkan/vulkan.h>
#include <cstddef>

#include "UniqueHandle.h"

namespace vko
{
    class Device;

    class DescriptorPool
    {
    public:
    	explicit DescriptorPool(Device const& device, std::size_t size);
    	~DescriptorPool();

        VkDescriptorPool getHandle() const { return m_handle; }
        std::size_t getSize() const { return m_size; }

    private:
        Device const& m_device;

        UniqueHandle<VkDescriptorPool> m_handle;
        std::size_t m_size;
    };
}
