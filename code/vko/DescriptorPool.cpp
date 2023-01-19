#include "DescriptorPool.h"

#include "vko/Assert.h"
#include "vko/Device.h"

#include "nstl/array.h"

#include <assert.h>

vko::DescriptorPool::DescriptorPool(Device const& device)
    : m_device(device)
{
    uint32_t const N = 1024;

    // TODO remove unnecessary pool allocations
    nstl::array poolSizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 4 * N},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT , 4 * N},
    };

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = N;

    VKO_VERIFY(vkCreateDescriptorPool(m_device.getHandle(), &poolCreateInfo, m_allocator, &m_handle.get()));
}

vko::DescriptorPool::~DescriptorPool()
{
    if (m_handle)
        vkDestroyDescriptorPool(m_device.getHandle(), m_handle, m_allocator);
}

nstl::optional<vko::DescriptorSets> vko::DescriptorPool::allocate(nstl::span<VkDescriptorSetLayout const> layouts)
{
    nstl::vector<VkDescriptorSet> descriptorSetHandles;
    descriptorSetHandles.resize(layouts.size());

    if (allocateRaw(layouts, descriptorSetHandles))
        return vko::DescriptorSets{ m_device, nstl::move(descriptorSetHandles) };

    return {};
}

bool vko::DescriptorPool::allocateRaw(nstl::span<VkDescriptorSetLayout const> layouts, nstl::span<VkDescriptorSet> descriptorSetHandles)
{
    assert(layouts.size() == descriptorSetHandles.size());

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_handle;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    VkResult result = vkAllocateDescriptorSets(m_device.getHandle(), &descriptorSetAllocInfo, descriptorSetHandles.data());

    if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
        return false;

    assert(result == VK_SUCCESS);
    return true;
}

void vko::DescriptorPool::reset()
{
    VKO_VERIFY(vkResetDescriptorPool(m_device.getHandle(), m_handle, 0));
}
