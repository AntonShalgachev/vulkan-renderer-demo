#pragma once

#include "framework.h"

namespace vkr
{
    class DescriptorPool;
    class DescriptorSetLayout;

    // TODO make it responsible for a single descriptor set
    class DescriptorSets
    {
    public:
    	DescriptorSets(std::size_t size, DescriptorSetLayout const& layout);
    	~DescriptorSets();

        std::vector<VkDescriptorSet> const& getHandles() const { return m_handles; }
        std::size_t getSize() const;

    private:
        std::unique_ptr<vkr::DescriptorPool> m_pool;
        std::vector<VkDescriptorSet> m_handles;
    };
}
