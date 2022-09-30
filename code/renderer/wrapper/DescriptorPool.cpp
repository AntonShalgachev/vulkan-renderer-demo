#include "DescriptorPool.h"

#include "Assert.h"
#include "Device.h"

#include <array>
#include <cassert>

vko::DescriptorPool::DescriptorPool(Device const& device)
    : m_device(device)
{
    uint32_t const N = 1024;

    std::array poolSizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 * N},
    };

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = N;

    VKO_ASSERT(vkCreateDescriptorPool(m_device.getHandle(), &poolCreateInfo, nullptr, &m_handle.get()));
}

vko::DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(m_device.getHandle(), m_handle, nullptr);
}

std::optional<vko::DescriptorSets> vko::DescriptorPool::allocate(std::span<VkDescriptorSetLayout const> layouts)
{
    std::vector<VkDescriptorSet> descriptorSetHandles = allocateRaw(layouts);

    if (descriptorSetHandles.empty())
        return {};

    return vko::DescriptorSets{ m_device, descriptorSetHandles };
}

std::vector<VkDescriptorSet> vko::DescriptorPool::allocateRaw(std::span<VkDescriptorSetLayout const> layouts)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_handle;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptorSetHandles;
    descriptorSetHandles.resize(layouts.size());

    VkResult result = vkAllocateDescriptorSets(m_device.getHandle(), &descriptorSetAllocInfo, descriptorSetHandles.data());

    if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
        return {};

    assert(result == VK_SUCCESS);

    return descriptorSetHandles;
}

void vko::DescriptorPool::reset()
{
    VKO_ASSERT(vkResetDescriptorPool(m_device.getHandle(), m_handle, 0));
}
