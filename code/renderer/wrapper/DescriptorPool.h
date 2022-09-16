#pragma once

#include <vulkan/vulkan.h>
#include <cstddef>
#include <optional>
#include <span>

#include "UniqueHandle.h"
#include "DescriptorSets.h" // TODO remove if possible

namespace vko
{
    class Device;

    class DescriptorPool
    {
    public:
    	explicit DescriptorPool(Device const& device);
    	~DescriptorPool();

        DescriptorPool(DescriptorPool const&) = default;
        DescriptorPool(DescriptorPool&&) = default;
        DescriptorPool& operator=(DescriptorPool const&) = default;
        DescriptorPool& operator=(DescriptorPool&&) = default;

        std::optional<DescriptorSets> allocate(std::span<VkDescriptorSetLayout const> layouts);
        void reset();

        VkDescriptorPool getHandle() const { return m_handle; }

    private:
        Device const& m_device;

        UniqueHandle<VkDescriptorPool> m_handle;
    };
}
