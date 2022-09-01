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

namespace
{
    std::size_t alignSize(std::size_t originalSize, std::size_t alignment)
    {
        std::size_t alignedSize = originalSize;
        if (alignment > 0)
            alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
        return alignedSize;
    }
}

vkr::ObjectInstance::ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, std::size_t uniformBufferSize, std::size_t swapchainImagesCount)
    : Object(app)
    , m_drawable(drawable)
    , m_transform(transform)
    , m_uniformBufferSize(uniformBufferSize)
    , m_alignedUniformBufferSize(alignSize(uniformBufferSize, app.getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment))
    , m_uniformBuffer(app.getDevice(), app.getPhysicalDevice(), m_alignedUniformBufferSize * swapchainImagesCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{

}

vkr::ObjectInstance::ObjectInstance(ObjectInstance&& rhs) = default;

vkr::ObjectInstance::~ObjectInstance() = default;

void vkr::ObjectInstance::copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    uint32_t offset = m_alignedUniformBufferSize * index;
    m_uniformBuffer.memory().copyFrom(sourcePointer, sourceSize, offset);
}
