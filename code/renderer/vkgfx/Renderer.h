#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "Handles.h"
#include "TestObject.h"
#include "PipelineKey.h"

namespace vko
{
    class Swapchain;
    class Framebuffer;
    class RenderPass;
    class Image;
    class ImageView;
    class DeviceMemory;
    class Window;
    struct DebugMessage;
}

namespace vkr
{
    class Application; // TODO don't use
}

namespace vkgfx
{
    struct RendererData;
    struct RendererFrameResources;
    class ResourceManager;
    struct TestObject;

    class Renderer
    {
    public:
        Renderer(std::string const& name, bool enableValidationLayers, vko::Window& window, std::function<void(vko::DebugMessage)> onDebugMessage = {});
        ~Renderer();

        ResourceManager& getResourceManager() const { return *m_resourceManager; }

        void setCameraTransform(TestCameraTransform transform);
        void setCameraParameters(TestCameraParameters parameters);
        void setLightParameters(TestLightParameters parameters);

        void addOneFrameTestObject(TestObject object);

        void waitIdle(); // TODO remove

        void draw();

    private:
        void onWindowResized();

        void createCameraResources();
        void recordCommandBuffer(std::size_t imageIndex, RendererFrameResources& frameResources);

        void updateCameraBuffer();

        void createSwapchain();
        void destroySwapchain();

        void recreateSwapchain();

    private:
        vko::Window& m_window;

        // TODO don't use unique_ptrs

        std::unique_ptr<vkr::Application> m_application; // TODO don't use
        std::unique_ptr<ResourceManager> m_resourceManager;

        std::unique_ptr<RendererData> m_data;

        std::unique_ptr<vko::Swapchain> m_swapchain;
        std::vector<std::unique_ptr<vko::Framebuffer>> m_swapchainFramebuffers;
        std::vector<std::unique_ptr<vko::ImageView>> m_swapchainImageViews;
        std::unique_ptr<vko::RenderPass> m_renderPass;
        std::unique_ptr<vko::Image> m_depthImage;
        std::unique_ptr<vko::DeviceMemory> m_depthImageMemory;
        std::unique_ptr<vko::ImageView> m_depthImageView;
        std::size_t m_width = 0;
        std::size_t m_height = 0;

        std::vector<RendererFrameResources> m_frameResources;
        std::size_t m_nextFrameResourcesIndex = 0;

        BufferHandle m_cameraBuffer;
        TestCameraTransform m_cameraTransform;
        TestCameraParameters m_cameraParameters;
        TestLightParameters m_lightParameters;
        UniformConfiguration m_frameUniformConfiguration;
        DescriptorSetLayoutHandle m_frameDescriptorSetLayout;

        std::vector<TestObject> m_oneFrameTestObjects;
    };
}
