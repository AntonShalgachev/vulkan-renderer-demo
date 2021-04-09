#include "PipelineLayout.h"
#include "DescriptorSetLayout.h"
#include "Device.h"
#include <array>
#include <stdexcept>

vkr::PipelineLayout::PipelineLayout(Application const& app, DescriptorSetLayout const& descriptorSetLayout) : Object(app)
{
    std::array setLayouts{ descriptorSetLayout.getHandle() };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(getDevice().getHandle(), &pipelineLayoutCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
}

vkr::PipelineLayout::~PipelineLayout()
{
    vkDestroyPipelineLayout(getDevice().getHandle(), m_handle, nullptr);
}
