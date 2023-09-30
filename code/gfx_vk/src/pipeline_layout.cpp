#include "pipeline_layout.h"

#include "context.h"

#include "vko/Assert.h"
#include "vko/Device.h"

gfx_vk::pipeline_layout::pipeline_layout(context& context, nstl::span<VkDescriptorSetLayout const> set_layouts)
    : m_context(context)
{
    VkPipelineLayoutCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
        .pSetLayouts = set_layouts.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    VKO_VERIFY(vkCreatePipelineLayout(m_context.get_device().getHandle(), &info, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::pipeline_layout::~pipeline_layout()
{
    if (!m_handle)
        return;

    vkDestroyPipelineLayout(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
    m_handle = nullptr;
}
