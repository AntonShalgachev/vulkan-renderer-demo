#include "PipelineLayout.h"

#include "vko/Assert.h"
#include "vko/Device.h"

vko::PipelineLayout::PipelineLayout(Device const& device, nstl::span<VkDescriptorSetLayout const> setLayouts, nstl::span<VkPushConstantRange const> pushConstantRanges)
    : m_device(device)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

    VKO_VERIFY(vkCreatePipelineLayout(m_device.getHandle(), &pipelineLayoutCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));

    m_descriptorSetLayouts = nstl::vector<VkDescriptorSetLayout>(setLayouts.begin(), setLayouts.end());
}

vko::PipelineLayout::~PipelineLayout()
{
    if (m_handle)
        vkDestroyPipelineLayout(m_device.getHandle(), m_handle, &m_allocator.getCallbacks());
}
