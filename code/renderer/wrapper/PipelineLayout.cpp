#include "PipelineLayout.h"
#include "DescriptorSetLayout.h"
#include "Device.h"
#include <array>
#include <stdexcept>

vkr::PipelineLayout::PipelineLayout(Application const& app, DescriptorSetLayout const& descriptorSetLayout, std::size_t pushConstantsSize) : Object(app)
{
    std::array setLayouts{ descriptorSetLayout.getHandle() };

    std::vector<VkPushConstantRange> pushConstantRanges;

    {
        VkPushConstantRange& range = pushConstantRanges.emplace_back();
        range.offset = 0;
        range.size = pushConstantsSize;
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

    if (vkCreatePipelineLayout(getDevice().getHandle(), &pipelineLayoutCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
}

vkr::PipelineLayout::~PipelineLayout()
{
    vkDestroyPipelineLayout(getDevice().getHandle(), m_handle, nullptr);
}
