#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
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
    	explicit DescriptorSets(Device const& device, DescriptorPool const& pool, DescriptorSetLayout const& layout);
    	~DescriptorSets();

        DescriptorSets(DescriptorSets const&) = delete;
        DescriptorSets(DescriptorSets&&) = delete;
        DescriptorSets& operator=(DescriptorSets const&) = delete;
        DescriptorSets& operator=(DescriptorSets&&) = delete;

        void update(std::size_t index, Buffer const& uniformBuffer, vkr::Texture const* texture, vkr::Texture const* normalMap);

        std::vector<VkDescriptorSet> const& getHandles() const { return m_handles; }
        std::size_t getSize() const;

    private:
        Device const& m_device;
        DescriptorPool const& m_pool;
        DescriptorSetLayout const& m_layout;

        std::vector<VkDescriptorSet> m_handles;
    };
}
