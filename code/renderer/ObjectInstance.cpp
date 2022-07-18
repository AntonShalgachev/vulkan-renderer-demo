#include "ObjectInstance.h"
#include "Utils.h"
#include "wrapper/DescriptorSets.h"
#include "wrapper/DeviceMemory.h"
#include "wrapper/Buffer.h"
#include "wrapper/PipelineLayout.h"
#include "wrapper/Swapchain.h"
#include "Material.h"
#include "Drawable.h"
#include "wrapper/DescriptorSetLayout.h"
#include "wrapper/Pipeline.h"
#include "wrapper/DescriptorPool.h"
#include "BufferWithMemory.h"

vkr::ObjectInstance::ObjectInstance(Application const& app, Drawable const* drawable, Transform const& transform, DescriptorSetLayout const& setLayout, VkDeviceSize uniformBufferSize)
    : Object(app)
    , m_drawable(drawable)
    , m_transform(transform)
    , m_setLayout(setLayout)
    , m_uniformBufferSize(uniformBufferSize)
{

}

vkr::ObjectInstance::~ObjectInstance() = default;

void vkr::ObjectInstance::onSwapchainCreated(Swapchain const& swapchain)
{
    std::size_t swapchainImageCount = swapchain.getImageCount();

    if (swapchainImageCount == m_currentImageCount)
        return;

    m_uniformBuffers.reserve(swapchainImageCount);

    for (size_t i = 0; i < swapchainImageCount; i++)
        m_uniformBuffers.emplace_back(getApp(), m_uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Material const& material = m_drawable->getMaterial();

    m_descriptorPool = std::make_unique<vkr::DescriptorPool>(getApp(), swapchainImageCount);
    m_descriptorSets = std::make_unique<vkr::DescriptorSets>(getApp(), *m_descriptorPool, m_setLayout);
    for (size_t i = 0; i < m_descriptorSets->getSize(); i++)
        m_descriptorSets->update(i, m_uniformBuffers[i].buffer(), material.getTexture().get(), material.getSampler().get());
}

void vkr::ObjectInstance::copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    m_uniformBuffers[index].memory().copyFrom(sourcePointer, sourceSize);
}

void vkr::ObjectInstance::bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, PipelineLayout const& pipelineLayout) const
{
    VkDescriptorSet handle = m_descriptorSets->getHandles()[imageIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), 0, 1, &handle, 0, nullptr);
}
