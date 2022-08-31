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
#include "wrapper/PhysicalDevice.h"

vkr::ObjectInstance::ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, vko::DescriptorSets descriptorSets, std::size_t uniformBufferSize, std::size_t swapchainImagesCount)
    : Object(app)
    , m_drawable(drawable)
    , m_transform(transform)
    , m_descriptorSets(std::move(descriptorSets))
    , m_uniformBufferSize(uniformBufferSize)
{
    vko::PhysicalDevice const& device = app.getPhysicalDevice();
    std::size_t minAlignment = device.getProperties().limits.minUniformBufferOffsetAlignment;

    m_alignedUniformBufferSize = m_uniformBufferSize;
    if (minAlignment > 0)
        m_alignedUniformBufferSize = (m_alignedUniformBufferSize + minAlignment - 1) & ~(minAlignment - 1);

    m_uniformBuffers.reserve(1);
    m_uniformBuffers.emplace_back(app.getDevice(), app.getPhysicalDevice(), m_alignedUniformBufferSize * swapchainImagesCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    Material const& material = m_drawable.getMaterial();

    m_descriptorSets.update(0, m_uniformBuffers[0].buffer(), m_uniformBufferSize, material.getTexture().get(), material.getNormalMap().get());
}

vkr::ObjectInstance::ObjectInstance(ObjectInstance&& rhs) = default;

vkr::ObjectInstance::~ObjectInstance() = default;

void vkr::ObjectInstance::copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    uint32_t offset = m_alignedUniformBufferSize * index;
    m_uniformBuffers[0].memory().copyFrom(sourcePointer, sourceSize, offset);
}

void vkr::ObjectInstance::bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, vko::PipelineLayout const& pipelineLayout) const
{
    VkDescriptorSet handle = m_descriptorSets.getHandle(0);
    uint32_t dynamicOffset = m_alignedUniformBufferSize * imageIndex;
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), 0, 1, &handle, 1, &dynamicOffset);
}
