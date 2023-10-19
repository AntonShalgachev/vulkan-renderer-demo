#include "pipeline_layout.h"

#include "context.h"

gfx_vk::pipeline_layout::pipeline_layout(context& context, nstl::span<VkDescriptorSetLayout const> layouts)
    : m_context(context)
    , m_layouts(layouts.begin(), layouts.end())
{
    VkPipelineLayoutCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    GFX_VK_VERIFY(vkCreatePipelineLayout(m_context.get_device_handle(), &info, &m_context.get_allocator(), &m_handle.get()));
}

gfx_vk::pipeline_layout::~pipeline_layout()
{
    if (!m_handle)
        return;

    vkDestroyPipelineLayout(m_context.get_device_handle(), m_handle, &m_context.get_allocator());
    m_handle = nullptr;
}
