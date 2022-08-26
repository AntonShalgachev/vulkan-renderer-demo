#include "DescriptorPool.h"
#include "Device.h"
#include <array>
#include <stdexcept>

#include "DescriptorSetLayout.h"

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

    if (vkCreateDescriptorPool(m_device.getHandle(), &poolCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool");
}

vko::DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(m_device.getHandle(), m_handle, nullptr);
}

std::optional<vko::DescriptorSets> vko::DescriptorPool::allocate(DescriptorSetLayout const& layout, std::size_t size)
{
    std::vector<VkDescriptorSetLayout> layouts(size, layout.getHandle());

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_handle;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptorSetHandles;
    descriptorSetHandles.resize(size);

    VkResult result = vkAllocateDescriptorSets(m_device.getHandle(), &descriptorSetAllocInfo, descriptorSetHandles.data());

    if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
        return {};

    if (result == VK_SUCCESS)
        return vko::DescriptorSets{ m_device, layout, descriptorSetHandles };

    throw std::runtime_error("failed to allocate descriptor sets!");
}
