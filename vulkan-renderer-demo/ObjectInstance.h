#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <memory>
#include <vector>

namespace vkr
{
    class Buffer;
    class DeviceMemory;
    class DescriptorSets;
    class Texture;
    class Sampler;
    class DescriptorSetLayout;
    class PipelineLayout;
    class SceneObject;
    class Swapchain;
    class Pipeline;

    class ObjectInstance : public Object
    {
    public:
        ObjectInstance(Application const& app, std::shared_ptr<SceneObject> const& sceneObject, DescriptorSetLayout const& setLayout, VkDeviceSize uniformBufferSize);

        SceneObject const& getSceneObject() const { return *m_sceneObject; }

        std::unique_ptr<vkr::DeviceMemory> const& getUniformBufferMemory(std::size_t imageIndex) const { return m_uniformBuffersMemory[imageIndex]; }
        std::unique_ptr<vkr::DescriptorSets> const& getDescriptorSets() const { return m_descriptorSets; }

        void onSwapchainCreated(Swapchain const& swapchain);

        void copyToUniformBuffer(std::size_t index, void const* sourcePointer, std::size_t sourceSize) const;
        void bindDescriptorSet(VkCommandBuffer commandBuffer, std::size_t imageIndex, PipelineLayout const& pipelineLayout) const;

        void setPipeline(std::unique_ptr<Pipeline> pipeline) { m_pipeline = std::move(pipeline); }
        bool hasPipeline() const { return m_pipeline != nullptr; }
        void bindPipeline(VkCommandBuffer commandBuffer) const;

    public:
        static void resetBoundDescriptorSet() { ms_boundDescriptorSet = VK_NULL_HANDLE; }

    private:
        std::shared_ptr<SceneObject> m_sceneObject;
        DescriptorSetLayout const& m_setLayout;
        VkDeviceSize m_uniformBufferSize;

        std::size_t m_currentImageCount = 0;
        std::vector<std::unique_ptr<vkr::Buffer>> m_uniformBuffers;
        std::vector<std::unique_ptr<vkr::DeviceMemory>> m_uniformBuffersMemory;
        std::unique_ptr<vkr::DescriptorSets> m_descriptorSets;

        std::unique_ptr<Pipeline> m_pipeline;

    private:
        static VkDescriptorSet ms_boundDescriptorSet;
    };
}
