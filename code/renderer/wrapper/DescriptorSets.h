#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstddef>

namespace vkr
{
    class Texture;
}

namespace vko
{
    class DescriptorPool;
    class Buffer;
    class Device;

    class DescriptorSets
    {
    public:
    	DescriptorSets(Device const& device, std::vector<VkDescriptorSet> handles);

        void update(std::size_t index, Buffer const& uniformBuffer, std::size_t bufferSize, vkr::Texture const* texture, vkr::Texture const* normalMap);

        VkDescriptorSet getHandle(std::size_t index) const { return m_handles[index]; }
        std::size_t getSize() const;

    private:
        Device const* m_device;

        std::vector<VkDescriptorSet> m_handles;
    };
}
