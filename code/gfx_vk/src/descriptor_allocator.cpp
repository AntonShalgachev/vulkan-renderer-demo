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
