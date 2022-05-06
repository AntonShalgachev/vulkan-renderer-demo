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

    // TODO make it responsible for a single descriptor set
    // Move DescriptorPool out of it
    class DescriptorSets : public Object
    {
    public:
    	explicit DescriptorSets(Application const& app, std::size_t size, DescriptorSetLayout const& layout);
    	~DescriptorSets();

        DescriptorSets(DescriptorSets const&) = delete;
        DescriptorSets(DescriptorSets&&) = delete;
        DescriptorSets& operator=(DescriptorSets const&) = delete;
        DescriptorSets& operator=(DescriptorSets&&) = delete;

        void update(std::size_t index, Buffer const& uniformBuffer, std::shared_ptr<Texture> const& texture, std::shared_ptr<Sampler> const& sampler);

        std::vector<VkDescriptorSet> const& getHandles() const { return m_handles; }
        std::size_t getSize() const;

    private:
        std::unique_ptr<vkr::DescriptorPool> m_pool;
        std::vector<VkDescriptorSet> m_handles;
    };
}
