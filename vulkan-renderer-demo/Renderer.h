#pragma once

#include <memory>

#include "Object.h"
#include "Semaphore.h"
#include "Fence.h"
#include <vector>
#include <functional>

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

    class Renderer : Object
    {
    public:
    	Renderer(Application const& app, vkr::DescriptorSetLayout const& descriptorSetLayout);
        ~Renderer();

        template<typename Func>
        void setUpdateUniformBufferCallback(Func&& func)
        {
            m_updateUniformBuffer = func;
        }

        template<typename Func>
        void setOnSwapchainCreatedCallback(Func&& func)
        {
            m_onSwapchainCreated = func;

            if (m_swapchain)
                onSwapchainCreated();
        }

        template<typename Func>
        void setWaitUntilWindowInForegroundCallback(Func&& func)
        {
            m_waitUntilWindowInForeground = func;
        }

        // TODO make generic
        void setObjects(Mesh const& mesh, ObjectInstance const& instance1, ObjectInstance const& instance2);

        void onFramebufferResized();
        void draw();

        float getAspect() const;

    private:
        void createSwapchain();
        void createSyncObjects();
        void createCommandBuffers(Mesh const& mesh, ObjectInstance const& instance1, ObjectInstance const& instance2);

        void recreateSwapchain();

        void onSwapchainCreated();

    private:
        DescriptorSetLayout const& m_descriptorSetLayout;

        std::unique_ptr<vkr::CommandBuffers> m_commandBuffers;

        std::unique_ptr<vkr::Swapchain> m_swapchain;
        std::unique_ptr<vkr::RenderPass> m_renderPass;
        std::unique_ptr<vkr::PipelineLayout> m_pipelineLayout;
        std::unique_ptr<vkr::Pipeline> m_pipeline;

        std::unique_ptr<vkr::Image> m_depthImage;
        std::unique_ptr<vkr::DeviceMemory> m_depthImageMemory;
        std::unique_ptr<vkr::ImageView> m_depthImageView;

        std::vector<vkr::Semaphore> m_imageAvailableSemaphores;
        std::vector<vkr::Semaphore> m_renderFinishedSemaphores;
        std::vector<vkr::Fence> m_inFlightFences;
        std::vector<vkr::Fence*> m_currentFences;

        std::size_t m_currentFrame = 0;
        bool m_framebufferResized = false;

        std::function<void(uint32_t)> m_updateUniformBuffer;
        std::function<void(uint32_t)> m_onSwapchainCreated;
        std::function<void()> m_waitUntilWindowInForeground;
    };
}
