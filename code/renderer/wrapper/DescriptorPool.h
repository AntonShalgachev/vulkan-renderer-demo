#pragma once

#include "UniqueHandle.h"
#include "DescriptorSets.h" // TODO remove if possible

#include "nstl/span.h"
#include "nstl/optional.h"

#include <vulkan/vulkan.h>

#include <cstddef>

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

        nstl::optional<DescriptorSets> allocate(nstl::span<VkDescriptorSetLayout const> layouts);
        nstl::vector<VkDescriptorSet> allocateRaw(nstl::span<VkDescriptorSetLayout const> layouts);
        void reset();

        VkDescriptorPool getHandle() const { return m_handle; }

    private:
        Device const& m_device;

        UniqueHandle<VkDescriptorPool> m_handle;
    };
}
