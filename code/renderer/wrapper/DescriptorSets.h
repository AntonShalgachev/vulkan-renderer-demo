#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstddef>

namespace vko
{
    class DescriptorPool;
    class Buffer;
    class Device;

    class DescriptorSets
    {
    public:
        struct UpdateConfig
        {
            struct SampledImage
            {
                std::size_t set = 0;
                std::size_t binding = 0;
                VkImageView imageView = VK_NULL_HANDLE;
                VkSampler sampler = VK_NULL_HANDLE;
            };

            struct Buffer
            {
                std::size_t set = 0;
                std::size_t binding = 0;
                VkBuffer buffer = VK_NULL_HANDLE;
                std::size_t offset = 0;
                std::size_t size = 0;
            };

            std::vector<SampledImage> images;
            std::vector<Buffer> buffers;
        };

    public:
    	DescriptorSets(Device const& device, std::vector<VkDescriptorSet> handles);

        void update(UpdateConfig const& updateConfig);

        VkDescriptorSet getHandle(std::size_t index) const { return m_handles[index]; }
        std::size_t getSize() const;

    private:
        Device const* m_device;

        std::vector<VkDescriptorSet> m_handles;
    };
}
