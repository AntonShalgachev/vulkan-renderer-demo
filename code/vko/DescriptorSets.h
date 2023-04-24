#pragma once

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

#include "stddef.h"

namespace vko
{
    class DescriptorPool;
    class Buffer;
    class Device;

    // Remove this class?
    class DescriptorSets
    {
    public:
        struct UpdateConfig
        {
            struct SampledImage
            {
                size_t set = 0;
                size_t binding = 0;
                VkImageView imageView = VK_NULL_HANDLE;
                VkSampler sampler = VK_NULL_HANDLE;
            };

            struct Buffer
            {
                size_t set = 0;
                size_t binding = 0;
                VkBuffer buffer = VK_NULL_HANDLE;
                size_t offset = 0;
                size_t size = 0;
            };

            nstl::vector<SampledImage> images;
            nstl::vector<Buffer> buffers;
        };

    public:
    	DescriptorSets(Device const& device, nstl::vector<VkDescriptorSet> handles);

        void update(UpdateConfig const& updateConfig);

        VkDescriptorSet getHandle(size_t index) const { return m_handles[index]; }
        size_t getSize() const;

    private:
        Device const* m_device;

        nstl::vector<VkDescriptorSet> m_handles;
    };
}
