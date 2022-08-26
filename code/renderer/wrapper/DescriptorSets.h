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
    class DescriptorSetLayout;
    class Buffer;
    class Device;

    class DescriptorSets
    {
    public:
    	DescriptorSets(Device const& device, DescriptorSetLayout const& layout, std::vector<VkDescriptorSet> handles);

        void update(std::size_t index, Buffer const& uniformBuffer, vkr::Texture const* texture, vkr::Texture const* normalMap);

        VkDescriptorSet getHandle(std::size_t index) const { return m_handles[index]; }
        std::size_t getSize() const;

    private:
        Device const* m_device;
        DescriptorSetLayout const* m_layout;

        std::vector<VkDescriptorSet> m_handles;
    };
}
