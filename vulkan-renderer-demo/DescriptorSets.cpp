#include "DescriptorSets.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"

vkr::DescriptorSets::DescriptorSets(std::size_t size, DescriptorSetLayout const& layout)
    : m_pool(std::make_unique<vkr::DescriptorPool>(size))
{
    std::vector<VkDescriptorSetLayout> layouts(size, layout.getHandle());

    VkDescriptorSetAllocateInfo descriptorSetallocInfo{};
    descriptorSetallocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetallocInfo.descriptorPool = m_pool->getHandle();
    descriptorSetallocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    descriptorSetallocInfo.pSetLayouts = layouts.data();

    m_handles.resize(size);
    if (vkAllocateDescriptorSets(temp::getDevice(), &descriptorSetallocInfo, m_handles.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");
}

vkr::DescriptorSets::~DescriptorSets()
{
    // no need to explicitly free descriptor sets
}

std::size_t vkr::DescriptorSets::getSize() const
{
    return m_pool->getSize();
}
