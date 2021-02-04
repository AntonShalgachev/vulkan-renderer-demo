#pragma once

#include <memory>

#include "Object.h"
#include "Semaphore.h"
#include "Fence.h"
#include <vector>
#include <functional>
#include "CommandBuffer.h"

namespace vkr
{
    class CommandBuffers;
    class Swapchain;
    class RenderPass;
    class PipelineLayout;
    class Pipeline;
    class Image;
    class DeviceMemory;
    class ImageView;
    class DescriptorSetLayout;
    class Mesh;
    class ObjectInstance;
    class SceneObject;
    class Shader;
    class CommandPool;
    class CommandBuffer;

    class Renderer : Object
    {
    public:
    	Renderer(Application const& app);
        ~Renderer();

        Swapchain const& getSwapchain() const { return *m_swapchain; }
        RenderPass const& getRenderPass() const { return *m_renderPass; }

        template<typename Func>
        void setWaitUntilWindowInForegroundCallback(Func&& func)
        {
            m_waitUntilWindowInForeground = func;
        }

        void addShader(Shader const& shader);

        void addObject(std::shared_ptr<SceneObject> const& object);
        void finalizeObjects();

        void onFramebufferResized();
        void draw();

        float getAspect() const;

    private:
        struct FrameResources
        {
            FrameResources(Application const& app);

            vkr::Semaphore imageAvailableSemaphore;
            vkr::Semaphore renderFinishedSemaphore;
            vkr::Fence inFlightFence;

            std::unique_ptr<vkr::CommandPool> commandPool;
            vkr::CommandBuffer commandBuffer;
        };

    private:
        void createSwapchain();
        void createSyncObjects();
        void createCommandBuffers();

        void recreateSwapchain();

        void onSwapchainCreated();

        void updateUniformBuffer(uint32_t currentImage);

    private:
        std::unique_ptr<vkr::Swapchain> m_swapchain;
        std::unique_ptr<vkr::RenderPass> m_renderPass;
        std::unique_ptr<vkr::PipelineLayout> m_pipelineLayout;

        std::unordered_map<Shader const*, std::unique_ptr<vkr::Pipeline>> m_pipelines;

        std::unique_ptr<vkr::Image> m_depthImage;
        std::unique_ptr<vkr::DeviceMemory> m_depthImageMemory;
        std::unique_ptr<vkr::ImageView> m_depthImageView;

        std::vector<FrameResources> m_frameResources;

        std::size_t m_nextFrameResourcesIndex = 0;
        bool m_framebufferResized = false;

        std::function<void()> m_waitUntilWindowInForeground;

        std::vector<std::unique_ptr<vkr::ObjectInstance>> m_sceneObjects;

        std::unique_ptr<vkr::DescriptorSetLayout> m_descriptorSetLayout;
    };
}
