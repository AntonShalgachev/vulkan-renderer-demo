#pragma once

#include "framework.h"
#include "Object.h"

namespace vkr
{
    class Buffer;
    class DeviceMemory;
    class DescriptorSets;
    class Texture;
    class Sampler;
    class DescriptorSetLayout;
    class PipelineLayout;

    class ObjectInstance : public Object
    {
    public:
    	ObjectInstance(Application const& app, VkDeviceSize uniformBufferSize, std::size_t swapchainImageCount, Texture const& texture, Sampler const& sampler, DescriptorSetLayout const& setLayout);

        std::unique_ptr<vkr::DeviceMemory> const& getUniformBufferMemory(std::size_t index) const { return m_uniformBuffersMemory[index]; }
        std::unique_ptr<vkr::DescriptorSets> const& getDescriptorSets() const { return m_descriptorSets; }

        void copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;
        void bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t index, PipelineLayout const& pipelineLayout) const;

    private:
        std::vector<std::unique_ptr<vkr::Buffer>> m_uniformBuffers;
        std::vector<std::unique_ptr<vkr::DeviceMemory>> m_uniformBuffersMemory;
        std::unique_ptr<vkr::DescriptorSets> m_descriptorSets;
    };
}
