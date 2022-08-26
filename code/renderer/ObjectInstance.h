#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <vector>
#include "wrapper/DescriptorSets.h"

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
    class BufferWithMemory;
    class Texture;
    class Drawable;
    class Transform;

    // TODO merge with Drawable
    // TODO don't to pass transform separately
    class ObjectInstance : public Object
    {
    public:
        ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, vko::DescriptorSets descriptorSets, VkDeviceSize uniformBufferSize, std::size_t swapchainImagesCount);
        ObjectInstance(ObjectInstance&& rhs);
        ~ObjectInstance();

        Drawable const& getDrawable() const { return m_drawable; }
        Transform const& getTransform() const { return m_transform; }

        void copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;
        void bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, vko::PipelineLayout const& pipelineLayout) const;

    private:
        Drawable const& m_drawable;
        Transform const& m_transform;

        std::vector<vkr::BufferWithMemory> m_uniformBuffers;
        vko::DescriptorSets m_descriptorSets;
    };
}
