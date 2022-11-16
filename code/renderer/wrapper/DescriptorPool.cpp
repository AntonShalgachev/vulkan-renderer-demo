#include "DescriptorPool.h"

#include "Assert.h"
#include "Device.h"

#include "nstl/array.h"

#include <cassert>

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

    VKO_ASSERT(vkCreateDescriptorPool(m_device.getHandle(), &poolCreateInfo, nullptr, &m_handle.get()));
}

vko::DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(m_device.getHandle(), m_handle, nullptr);
}

std::optional<vko::DescriptorSets> vko::DescriptorPool::allocate(nstl::span<VkDescriptorSetLayout const> layouts)
{
    nstl::vector<VkDescriptorSet> descriptorSetHandles = allocateRaw(layouts);

    if (descriptorSetHandles.empty())
        return {};

    return vko::DescriptorSets{ m_device, nstl::move(descriptorSetHandles) };
}

nstl::vector<VkDescriptorSet> vko::DescriptorPool::allocateRaw(nstl::span<VkDescriptorSetLayout const> layouts)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_handle;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    nstl::vector<VkDescriptorSet> descriptorSetHandles;
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
