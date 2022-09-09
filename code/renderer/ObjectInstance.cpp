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

    vkr::FancyBuffer createFancyBuffer(vkr::Application const& app, std::size_t chunkSize, std::size_t duplicationCount)
    {
        std::size_t alignment = app.getPhysicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
        std::size_t alignedChunkSize = alignSize(chunkSize, alignment);
        std::size_t totalSize = alignedChunkSize * duplicationCount;

        return vkr::FancyBuffer{
            .buffer{app.getDevice(), app.getPhysicalDevice(), totalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
            .chunkSize = chunkSize,
            .alignedChunkSize = alignedChunkSize,
            .totalSize = totalSize,
        };
    }
}

vkr::ObjectInstance::ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, std::size_t objectUniformBufferSize, std::size_t materialUniformBufferSize, std::size_t frameUniformBufferSize, std::size_t swapchainImagesCount)
    : Object(app)
    , m_drawable(drawable)
    , m_transform(transform)
    , m_objectUniformBuffer(createFancyBuffer(app, objectUniformBufferSize, swapchainImagesCount))
    , m_materialUniformBuffer(createFancyBuffer(app, materialUniformBufferSize, swapchainImagesCount))
    , m_frameUniformBuffer(createFancyBuffer(app, frameUniformBufferSize, swapchainImagesCount))
{

}

vkr::ObjectInstance::ObjectInstance(ObjectInstance&& rhs) = default;

vkr::ObjectInstance::~ObjectInstance() = default;

std::vector<uint32_t> vkr::ObjectInstance::getBufferOffsets(std::size_t imageIndex) const
{
    return {
        static_cast<uint32_t>(m_objectUniformBuffer.alignedChunkSize * imageIndex),
        static_cast<uint32_t>(m_materialUniformBuffer.alignedChunkSize * imageIndex),
        static_cast<uint32_t>(m_frameUniformBuffer.alignedChunkSize * imageIndex),
    };
}

void vkr::ObjectInstance::copyToObjectUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    return copyToUniformBuffer(m_objectUniformBuffer, index, sourcePointer, sourceSize);
}

void vkr::ObjectInstance::copyToMaterialUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    return copyToUniformBuffer(m_materialUniformBuffer, index, sourcePointer, sourceSize);
}

void vkr::ObjectInstance::copyToFrameUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    return copyToUniformBuffer(m_frameUniformBuffer, index, sourcePointer, sourceSize);
}

void vkr::ObjectInstance::copyToUniformBuffer(FancyBuffer const& buffer, std::size_t index, void const* sourcePointer, std::size_t sourceSize) const
{
    uint32_t offset = buffer.alignedChunkSize * index;
    buffer.buffer.memory().copyFrom(sourcePointer, sourceSize, offset);
}
