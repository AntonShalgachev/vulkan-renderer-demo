#include "ObjectInstance.h"
#include "Utils.h"
#include "DescriptorSets.h"
#include "DeviceMemory.h"
#include "Buffer.h"
#include "PipelineLayout.h"
#include "Swapchain.h"
#include "Material.h"
#include "SceneObject.h"
#include "DescriptorSetLayout.h"
#include "Pipeline.h"

VkDescriptorSet vkr::ObjectInstance::ms_boundDescriptorSet = VK_NULL_HANDLE;

vkr::ObjectInstance::ObjectInstance(Application const& app, std::shared_ptr<SceneObject> const& sceneObject, DescriptorSetLayout const& setLayout, VkDeviceSize uniformBufferSize)
    : Object(app)
    , m_sceneObject(sceneObject)
    , m_setLayout(setLayout)
    , m_uniformBufferSize(uniformBufferSize)
{

}

void vkr::ObjectInstance::onSwapchainCreated(Swapchain const& swapchain)
{
    std::size_t swapchainImageCount = swapchain.getImageCount();

    if (swapchainImageCount == m_currentImageCount)
        return;

    m_uniformBuffers.resize(swapchainImageCount);
    m_uniformBuffersMemory.resize(swapchainImageCount);

    for (size_t i = 0; i < swapchainImageCount; i++)
        vkr::utils::createBuffer(getApp(), m_uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

    if (!m_sceneObject->isValid())
        return;

    Material const& material = m_sceneObject->getMaterial();

    m_descriptorSets = std::make_unique<vkr::DescriptorSets>(getApp(), swapchainImageCount, m_setLayout);
    for (size_t i = 0; i < m_descriptorSets->getSize(); i++)
        m_descriptorSets->update(i, *m_uniformBuffers[i], material.getTexture(), material.getSampler());
}

void vkr::ObjectInstance::copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    m_uniformBuffersMemory[index]->copyFrom(sourcePointer, sourceSize);
}

void vkr::ObjectInstance::bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, PipelineLayout const& pipelineLayout) const
{
    VkDescriptorSet handle = m_descriptorSets->getHandles()[imageIndex];
    if (ms_boundDescriptorSet == handle)
        return;

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), 0, 1, &handle, 0, nullptr);
    ms_boundDescriptorSet = handle;
}

void vkr::ObjectInstance::bindPipeline(VkCommandBuffer commandBuffer) const
{
    m_pipeline->bind(commandBuffer);
}
