#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <vector>
#include "wrapper/DescriptorSets.h"
#include "BufferWithMemory.h"

namespace vko
{
    class DescriptorSets;
    class Sampler;
    class DescriptorSetLayout;
    class PipelineLayout;
    class DescriptorPool;
}

namespace vkr
{
    class Texture;
    class Drawable;
    class Transform;

    // TODO remove or rename it
    struct FancyBuffer
    {
        BufferWithMemory buffer;
        std::size_t chunkSize = 0;
        std::size_t alignedChunkSize = 0;
        std::size_t totalSize = 0;
    };

    // TODO merge with Drawable
    // TODO don't to pass transform separately
    class ObjectInstance : public Object
    {
    public:
        ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, std::size_t objectUniformBufferSize, std::size_t materialUniformBufferSize, std::size_t frameUniformBufferSize, std::size_t swapchainImagesCount);
        ObjectInstance(ObjectInstance&& rhs);
        ~ObjectInstance();

        Drawable const& getDrawable() const { return m_drawable; }
        Transform const& getTransform() const { return m_transform; }

        vkr::BufferWithMemory const& getObjectBuffer() const { return m_objectUniformBuffer.buffer; }
        vkr::BufferWithMemory const& getMaterialBuffer() const { return m_materialUniformBuffer.buffer; }
        vkr::BufferWithMemory const& getFrameBuffer() const { return m_frameUniformBuffer.buffer; }

        std::vector<uint32_t> getBufferOffsets(std::size_t imageIndex) const;

        void copyToObjectUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;
        void copyToMaterialUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;
        void copyToFrameUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;

    private:
        void copyToUniformBuffer(FancyBuffer const& buffer, std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;

    private:
        Drawable const& m_drawable;
        Transform const& m_transform;

        FancyBuffer m_objectUniformBuffer;
        FancyBuffer m_materialUniformBuffer;
        FancyBuffer m_frameUniformBuffer;
    };
}
