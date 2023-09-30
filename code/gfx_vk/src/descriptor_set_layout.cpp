#include "descriptor_set_layout.h"

#include "context.h"

#include "vko/Assert.h"
#include "vko/Device.h"

#include "nstl/static_vector.h"

gfx_vk::descriptor_set_layout::descriptor_set_layout(context& context, gfx::uniform_group_configuration const& params)
    : m_context(context)
{
    // TODO pass configuration externally

    nstl::static_vector<VkDescriptorSetLayoutBinding, 4> bindings;

    if (params.has_buffer)
    {
        VkDescriptorSetLayoutBinding& desc = bindings.emplace_back();
        desc.binding = 0;
        desc.descriptorCount = 1;
        desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        desc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT; // TODO specify dynamically
        desc.pImmutableSamplers = nullptr;
    }

    if (params.has_albedo_texture)
    {
        VkDescriptorSetLayoutBinding& desc = bindings.emplace_back();
        desc.binding = 1;
        desc.descriptorCount = 1;
        desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        desc.pImmutableSamplers = nullptr;
    }

    if (params.has_normal_map)
    {
        VkDescriptorSetLayoutBinding& desc = bindings.emplace_back();
        desc.binding = 2;
        desc.descriptorCount = 1;
        desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        desc.pImmutableSamplers = nullptr;
    }

    if (params.has_shadow_map)
    {
        VkDescriptorSetLayoutBinding& desc = bindings.emplace_back();
        desc.binding = 3;
        desc.descriptorCount = 1;
        desc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        desc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        desc.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    VKO_VERIFY(vkCreateDescriptorSetLayout(m_context.get_device().getHandle(), &info, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::descriptor_set_layout::~descriptor_set_layout()
{
    if (!m_handle)
        return;

    vkDestroyDescriptorSetLayout(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}
