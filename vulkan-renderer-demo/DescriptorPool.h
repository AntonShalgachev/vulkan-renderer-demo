#pragma once

#include <vulkan/vulkan.h>
#include <cstddef>

#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class DescriptorPool : public Object
    {
    public:
    	explicit DescriptorPool(Application const& app, std::size_t size);
    	~DescriptorPool();

        DescriptorPool(DescriptorPool const&) = default;
        DescriptorPool(DescriptorPool&&) = default;
        DescriptorPool& operator=(DescriptorPool const&) = default;
        DescriptorPool& operator=(DescriptorPool&&) = default;

        VkDescriptorPool getHandle() const { return m_handle; }
        std::size_t getSize() const { return m_size; }

    private:
        UniqueHandle<VkDescriptorPool> m_handle;
        std::size_t m_size;
    };
}
