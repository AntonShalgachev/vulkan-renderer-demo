#include "DescriptorPool.h"
#include "Device.h"
#include <array>

vkr::DescriptorPool::DescriptorPool(Application const& app, std::size_t size) : Object(app)
{
    m_size = size;

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(size);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(size);

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = static_cast<uint32_t>(size);

    if (vkCreateDescriptorPool(getDevice().getHandle(), &poolCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool");
}

vkr::DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(getDevice().getHandle(), m_handle, nullptr);
}
