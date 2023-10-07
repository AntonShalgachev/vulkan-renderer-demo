#include "descriptor_allocator.h"

#include "context.h"

#include "vko/Device.h"
#include "vko/Assert.h"

#include "nstl/array.h"

gfx_vk::descriptor_allocator::descriptor_allocator(context& context, descriptors_config const& config)
    : m_context(context)
{
    nstl::array pool_sizes = {
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, config.max_descriptors_per_type_per_pool},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, config.max_descriptors_per_type_per_pool},
    };

    VkDescriptorPoolCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = config.max_sets_per_pool,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    VKO_VERIFY(vkCreateDescriptorPool(m_context.get_device().getHandle(), &info, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::descriptor_allocator::~descriptor_allocator()
{
    if (!m_handle)
        return;

    vkDestroyDescriptorPool(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}

bool gfx_vk::descriptor_allocator::allocate(nstl::span<VkDescriptorSetLayout const> layouts, nstl::span<VkDescriptorSet> handles)
{
    assert(layouts.size() == handles.size());

    VkDescriptorSetAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_handle,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };

    VkResult result = vkAllocateDescriptorSets(m_context.get_device().getHandle(), &info, handles.data());

    if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
        return false;

    assert(result == VK_SUCCESS);
    return true;
}
