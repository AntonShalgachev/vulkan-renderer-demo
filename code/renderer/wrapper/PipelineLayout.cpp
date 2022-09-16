#include "PipelineLayout.h"

#include "Device.h"
#include <stdexcept>

vko::PipelineLayout::PipelineLayout(Device const& device, std::span<VkDescriptorSetLayout const> setLayouts, std::span<VkPushConstantRange const> pushConstantRanges)
    : m_device(device)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

    if (vkCreatePipelineLayout(m_device.getHandle(), &pipelineLayoutCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");

    m_descriptorSetLayouts.assign(setLayouts.begin(), setLayouts.end());
}

vko::PipelineLayout::~PipelineLayout()
{
    vkDestroyPipelineLayout(m_device.getHandle(), m_handle, nullptr);
}
