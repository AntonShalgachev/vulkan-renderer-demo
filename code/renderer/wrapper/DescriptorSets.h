#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <cstddef>
#include "Object.h"

namespace vkr
{
    class DescriptorPool;
    class DescriptorSetLayout;
    class Buffer;
    class Texture;
    class Sampler;

    class DescriptorSets : public Object
    {
    public:
    	explicit DescriptorSets(Application const& app, DescriptorPool const& pool, DescriptorSetLayout const& layout);
    	~DescriptorSets();

        DescriptorSets(DescriptorSets const&) = delete;
        DescriptorSets(DescriptorSets&&) = delete;
        DescriptorSets& operator=(DescriptorSets const&) = delete;
        DescriptorSets& operator=(DescriptorSets&&) = delete;

        void update(std::size_t index, Buffer const& uniformBuffer, Texture const* texture, Texture const* normalMap, Sampler const* sampler);

        std::vector<VkDescriptorSet> const& getHandles() const { return m_handles; }
        std::size_t getSize() const;

    private:
        DescriptorPool const& m_pool;
        DescriptorSetLayout const& m_layout;
        std::vector<VkDescriptorSet> m_handles;
    };
}
