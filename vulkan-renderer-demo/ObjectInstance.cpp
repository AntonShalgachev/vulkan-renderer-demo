#include "ObjectInstance.h"
#include "Utils.h"
#include "DescriptorSets.h"
#include "DeviceMemory.h"
#include "Buffer.h"
#include "PipelineLayout.h"

vkr::ObjectInstance::ObjectInstance(VkDeviceSize uniformBufferSize, std::size_t swapchainImageCount, Texture const& texture, Sampler const& sampler, DescriptorSetLayout const& setLayout)
{
    m_uniformBuffers.resize(swapchainImageCount);
    m_uniformBuffersMemory.resize(swapchainImageCount);

    for (size_t i = 0; i < swapchainImageCount; i++)
        vkr::utils::createBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

    m_descriptorSets = std::make_unique<vkr::DescriptorSets>(swapchainImageCount, setLayout);
    for (size_t i = 0; i < m_descriptorSets->getSize(); i++)
        m_descriptorSets->update(i, *m_uniformBuffers[i], texture, sampler);
}

void vkr::ObjectInstance::copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    m_uniformBuffersMemory[index]->copyFrom(sourcePointer, sourceSize);
}

void vkr::ObjectInstance::bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t index, PipelineLayout const& pipelineLayout) const
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), 0, 1, &m_descriptorSets->getHandles()[index], 0, nullptr);
}
