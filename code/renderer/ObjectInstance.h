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

    // TODO merge with Drawable
    // TODO don't to pass transform separately
    class ObjectInstance : public Object
    {
    public:
        ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, std::size_t uniformBufferSize, std::size_t swapchainImagesCount);
        ObjectInstance(ObjectInstance&& rhs);
        ~ObjectInstance();

        Drawable const& getDrawable() const { return m_drawable; }
        Transform const& getTransform() const { return m_transform; }

        vkr::BufferWithMemory const& getBuffer() const { return m_uniformBuffer; }
        std::size_t getAlignedUniformBufferSize() const { return m_alignedUniformBufferSize; }

        void copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;

    private:
        Drawable const& m_drawable;
        Transform const& m_transform;

        std::size_t m_uniformBufferSize;
        std::size_t m_alignedUniformBufferSize;
        vkr::BufferWithMemory m_uniformBuffer;
    };
}
