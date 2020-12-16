#include "PipelineLayout.h"
#include "DescriptorSetLayout.h"
#include "Device.h"

vkr::PipelineLayout::PipelineLayout(Application const& app, DescriptorSetLayout const& descriptorSetLayout) : Object(app)
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout.getHandle();
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(getDevice().getHandle(), &pipelineLayoutCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
}

vkr::PipelineLayout::~PipelineLayout()
{
    vkDestroyPipelineLayout(getDevice().getHandle(), m_handle, nullptr);
}
