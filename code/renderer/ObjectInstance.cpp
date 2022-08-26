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
#include "Application.h"

vkr::ObjectInstance::ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, vko::DescriptorSetLayout const& setLayout, VkDeviceSize uniformBufferSize, std::size_t swapchainImagesCount)
    : Object(app)
    , m_drawable(drawable)
    , m_transform(transform)
{
    m_uniformBuffers.reserve(swapchainImagesCount);

    for (size_t i = 0; i < swapchainImagesCount; i++)
        m_uniformBuffers.emplace_back(app.getDevice(), app.getPhysicalDevice(), uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Material const& material = m_drawable.getMaterial();

    m_descriptorPool = std::make_unique<vko::DescriptorPool>(getDevice(), swapchainImagesCount);
    m_descriptorSets = std::make_unique<vko::DescriptorSets>(getDevice(), *m_descriptorPool, setLayout);
    for (size_t i = 0; i < m_descriptorSets->getSize(); i++)
        m_descriptorSets->update(i, m_uniformBuffers[i].buffer(), material.getTexture().get(), material.getNormalMap().get());
}

vkr::ObjectInstance::ObjectInstance(ObjectInstance&& rhs) = default;

vkr::ObjectInstance::~ObjectInstance() = default;

void vkr::ObjectInstance::copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    m_uniformBuffers[index].memory().copyFrom(sourcePointer, sourceSize);
}

void vkr::ObjectInstance::bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, vko::PipelineLayout const& pipelineLayout) const
{
    VkDescriptorSet handle = m_descriptorSets->getHandles()[imageIndex];
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), 0, 1, &handle, 0, nullptr);
}
