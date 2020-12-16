#pragma once

#include "framework.h"
#include "Object.h"

namespace vkr
{
    class DescriptorPool : public Object
    {
    public:
    	explicit DescriptorPool(std::size_t size);
    	~DescriptorPool();

        DescriptorPool(DescriptorPool const&) = delete;
        DescriptorPool(DescriptorPool&&) = delete;
        DescriptorPool& operator=(DescriptorPool const&) = delete;
        DescriptorPool& operator=(DescriptorPool&&) = delete;

        VkDescriptorPool const& getHandle() const { return m_handle; }
        std::size_t getSize() const { return m_size; }

    private:
    	VkDescriptorPool m_handle = VK_NULL_HANDLE;
        std::size_t m_size;
    };
}
