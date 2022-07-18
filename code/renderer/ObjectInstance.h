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

    // TODO merge with Drawable
    class ObjectInstance : public Object
    {
    public:
        ObjectInstance(Application const& app, std::shared_ptr<Drawable> const& drawable, DescriptorSetLayout const& setLayout, VkDeviceSize uniformBufferSize);
        ~ObjectInstance();

        Drawable const& getDrawable() const { return *m_drawable; }

        std::unique_ptr<vkr::DescriptorSets> const& getDescriptorSets() const { return m_descriptorSets; }

        void onSwapchainCreated(Swapchain const& swapchain);

        void copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;
        void bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, PipelineLayout const& pipelineLayout) const;

    private:
        std::shared_ptr<Drawable> m_drawable;
        DescriptorSetLayout const& m_setLayout;
        VkDeviceSize m_uniformBufferSize;

        std::size_t m_currentImageCount = 0;
        std::vector<vkr::BufferWithMemory> m_uniformBuffers;

        std::unique_ptr<vkr::DescriptorPool> m_descriptorPool;
        std::unique_ptr<vkr::DescriptorSets> m_descriptorSets;
    };
}
