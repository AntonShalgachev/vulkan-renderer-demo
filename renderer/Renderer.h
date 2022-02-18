#pragma once

#include <memory>

#include "Object.h"
#include "Semaphore.h"
#include "Fence.h"
#include <vector>
#include <functional>
#include "CommandBuffer.h"
#include "Timer.h"
#include "Camera.h"
#include "Light.h"

// TODO maybe remove?
#include "Shader.h"

#include <unordered_map>
#include "PipelineConfiguration.h"

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
    class CommandPool;
    class CommandBuffer;
    class VertexLayoutDescriptions;

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

        void addObject(std::shared_ptr<SceneObject> const& object);
        void setLight(std::shared_ptr<Light> const& light) { m_light = light; }

        void setCamera(std::shared_ptr<SceneObject> const& cameraObject)
        {
            m_activeCameraObject = cameraObject;
            updateCameraAspect();
        }

        void onFramebufferResized();
        void draw();

        float getAspect() const;
        float getLastFenceTime() const { return m_lastFenceTime; }
        float getCumulativeFenceTime() const { return m_cumulativeFenceTime; }
        void resetCumulativeFenceTime() { m_cumulativeFenceTime = 0.0f; }

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
        void recordCommandBuffer(std::size_t imageIndex, FrameResources const& frameResources);
        std::unique_ptr<Pipeline> createPipeline(PipelineConfiguration const& configuration);

        void recreateSwapchain();

        void onSwapchainCreated();

        void updateUniformBuffer(uint32_t currentImage);

        void updateCameraAspect();

    private:
        std::shared_ptr<SceneObject> m_activeCameraObject;

        std::unique_ptr<vkr::Swapchain> m_swapchain;
        std::unique_ptr<vkr::RenderPass> m_renderPass;
        std::unique_ptr<vkr::PipelineLayout> m_pipelineLayout;

        std::unique_ptr<vkr::Image> m_depthImage;
        std::unique_ptr<vkr::DeviceMemory> m_depthImageMemory;
        std::unique_ptr<vkr::ImageView> m_depthImageView;

        std::vector<FrameResources> m_frameResources;

        std::size_t m_nextFrameResourcesIndex = 0;
        bool m_framebufferResized = false;

        std::function<void()> m_waitUntilWindowInForeground;

        std::vector<std::unique_ptr<vkr::ObjectInstance>> m_sceneObjects;
        std::shared_ptr<Light> m_light;

        std::unique_ptr<vkr::DescriptorSetLayout> m_descriptorSetLayout;

        std::unordered_map<PipelineConfiguration, std::unique_ptr<Pipeline>> m_pipelines;

        vkr::Timer m_fenceTimer;
        float m_lastFenceTime = 0.0f;
        float m_cumulativeFenceTime = 0.0f;
    };
}