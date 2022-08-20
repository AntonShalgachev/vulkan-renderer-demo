#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <vector>

namespace vkr
{
    class BufferWithMemory;
    class DescriptorSets;
    class Texture;
    class Sampler;
    class DescriptorSetLayout;
    class PipelineLayout;
    class Drawable;
    class Swapchain;
    class DescriptorPool;
    class Transform;

    // TODO merge with Drawable
    // TODO don't to pass transform separately
    class ObjectInstance : public Object
    {
    public:
        ObjectInstance(Application const& app, Drawable const& drawable, Transform const& transform, DescriptorSetLayout const& setLayout, VkDeviceSize uniformBufferSize, std::size_t swapchainImagesCount);
        ObjectInstance(ObjectInstance&& rhs);
        ~ObjectInstance();

        Drawable const& getDrawable() const { return m_drawable; }
        Transform const& getTransform() const { return m_transform; }

        void copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;
        void bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, PipelineLayout const& pipelineLayout) const;

    private:
        Drawable const& m_drawable;
        Transform const& m_transform;

        std::vector<vkr::BufferWithMemory> m_uniformBuffers;
        std::unique_ptr<vkr::DescriptorPool> m_descriptorPool;
        std::unique_ptr<vkr::DescriptorSets> m_descriptorSets;
    };
}
