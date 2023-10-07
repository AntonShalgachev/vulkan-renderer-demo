#include "descriptor_set_layout.h"

#include "context.h"
#include "conversions.h"

#include "vko/Assert.h"
#include "vko/Device.h"

#include "nstl/static_vector.h"

gfx_vk::descriptor_set_layout::descriptor_set_layout(context& context, gfx::descriptorgroup_layout_view const& layout)
    : m_context(context)
    , m_layout(gfx::descriptorgroup_layout_storage::from_view(layout))
{
    nstl::vector<VkDescriptorSetLayoutBinding> bindings;

    for (gfx::descriptor_layout_entry const& entry : layout.entries)
    {
        bindings.push_back({
            .binding = static_cast<uint32_t>(entry.location),
            .descriptorType = utils::get_descriptor_type(entry.type),
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS, // TODO restrict to only necessary stages
            .pImmutableSamplers = nullptr, // TODO make use of immutable samplers
        });
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
