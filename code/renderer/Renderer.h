#pragma once

#include <memory>

#include "Object.h"
#include "wrapper/Semaphore.h"
#include "wrapper/Fence.h"
#include <vector>
#include <functional>
#include "CommandBuffer.h"
#include "Timer.h"
#include "Camera.h"
#include "Light.h"
#include "wrapper/CommandBuffers.h"
#include "wrapper/CommandPool.h"

#include <unordered_map>
#include <map>
#include "PipelineConfiguration.h"
#include "wrapper/DescriptorSetLayout.h" // TODO only vko::DescriptorSetConfiguration is needed

namespace vko
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
    class Framebuffer;
    class Sampler;
    class DescriptorPool;
}

namespace vkr
{
    class Mesh;
    class ObjectInstance;
    class Drawable;
    class CommandBuffer;
    class VertexLayoutDescriptions;
    class SceneObject;
    class Texture;
    class BufferWithMemory;

    struct OneFrameBoxResources
    {
        std::unique_ptr<vkr::BufferWithMemory> bufferWithMemory;
        std::unique_ptr<vkr::Mesh> mesh;

        std::unique_ptr<vko::PipelineLayout> pipelineLayout;
        std::unique_ptr<vko::Pipeline> pipeline;
    };

    struct OneFrameBoxInstance
    {
        glm::mat4 model;
        glm::vec3 color;
    };

    class Renderer : Object
    {
    public:
    	Renderer(Application const& app);
        ~Renderer();

        vko::Swapchain const& getSwapchain() const { return *m_swapchain; }
        vko::RenderPass const& getRenderPass() const { return *m_renderPass; }

        void setWaitUntilWindowInForegroundCallback(std::function<void()> func);

        void addDrawable(SceneObject const& drawableObject);
        void setLight(std::shared_ptr<Light> const& light) { m_light = light; }
        void clearObjects();

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

        void setOneFrameBoxes(std::vector<OneFrameBoxInstance> instances) { m_oneFrameBoxInstances = std::move(instances); }

    private:
        struct FrameResources
        {
            vko::Semaphore imageAvailableSemaphore;
            vko::Semaphore renderFinishedSemaphore;
            vko::Fence inFlightFence;

            std::vector<vko::DescriptorPool> descriptorPools;

            vko::CommandPool commandPool;
            vko::CommandBuffers commandBuffers;
        };

        struct UniformResources
        {
            std::vector<std::unique_ptr<vko::DescriptorSetLayout>> descriptorSetLayouts;
            std::unique_ptr<vko::PipelineLayout> pipelineLayout;
        };

    private:
        void createSwapchain();
        void createSyncObjects();
        void recordCommandBuffer(std::size_t imageIndex, FrameResources& frameResources);
        std::unique_ptr<vko::Pipeline> createPipeline(PipelineConfiguration const& configuration);

        void destroySwapchain();
        void recreateSwapchain();

        void onSwapchainCreated();

        void updateCameraAspect();

        UniformResources const& getUniformResources(std::vector<vko::DescriptorSetConfiguration> const& configs);

    private:
        std::shared_ptr<SceneObject> m_activeCameraObject;

        std::unique_ptr<vko::Swapchain> m_swapchain;
        std::vector<std::unique_ptr<vko::Framebuffer>> m_swapchainFramebuffers;
        std::vector<std::unique_ptr<vko::ImageView>> m_swapchainImageViews;
        std::unique_ptr<vko::RenderPass> m_renderPass;

        std::unique_ptr<vko::Image> m_depthImage;
        std::unique_ptr<vko::DeviceMemory> m_depthImageMemory;
        std::unique_ptr<vko::ImageView> m_depthImageView;

        std::vector<FrameResources> m_frameResources;

        std::size_t m_nextFrameResourcesIndex = 0;
        bool m_framebufferResized = false;

        std::function<void()> m_waitUntilWindowInForeground;

        std::vector<ObjectInstance> m_drawableInstances;
        std::shared_ptr<Light> m_light;

        std::map<std::vector<vko::DescriptorSetConfiguration>, UniformResources> m_uniformResources;

        std::unordered_map<PipelineConfiguration, std::unique_ptr<vko::Pipeline>> m_pipelines;

        vkr::Timer m_fenceTimer;
        float m_lastFenceTime = 0.0f;
        float m_cumulativeFenceTime = 0.0f;

        // TODO move somewhere else
        OneFrameBoxResources m_oneFrameBoxResources;
        std::vector<OneFrameBoxInstance> m_oneFrameBoxInstances;
    };
}
