#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

#include <span>
#include <vector>

namespace vko
{
    class Device;

    class PipelineLayout
    {
    public:
    	explicit PipelineLayout(Device const& device, std::span<VkDescriptorSetLayout const> setLayouts, std::span<VkPushConstantRange const> pushConstantRanges);
    	~PipelineLayout();

        PipelineLayout(PipelineLayout const&) = default;
        PipelineLayout(PipelineLayout&&) = default;
        PipelineLayout& operator=(PipelineLayout const&) = default;
        PipelineLayout& operator=(PipelineLayout&&) = default;

        VkPipelineLayout getHandle() const { return m_handle; }
        std::span<VkDescriptorSetLayout const> getDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

    private:
        Device const& m_device;
    	UniqueHandle<VkPipelineLayout> m_handle;

        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    };
}
