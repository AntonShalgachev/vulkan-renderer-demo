#pragma once

#include "vkgfx/Handles.h"
#include "vkgfx/TestObject.h"
#include "vkgfx/PipelineKey.h"

#include "nstl/vector.h"
#include "nstl/function.h"
#include "nstl/unique_ptr.h"

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
        Renderer(char const* name, bool enableValidationLayers, vko::Window& window, nstl::function<void(vko::DebugMessage)> onDebugMessage = {});
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

        nstl::unique_ptr<vkr::Application> m_application; // TODO don't use
        nstl::unique_ptr<ResourceManager> m_resourceManager;

        nstl::unique_ptr<RendererData> m_data;

        nstl::unique_ptr<vko::Swapchain> m_swapchain;
        nstl::vector<nstl::unique_ptr<vko::Framebuffer>> m_swapchainFramebuffers;
        nstl::vector<nstl::unique_ptr<vko::ImageView>> m_swapchainImageViews;
        nstl::unique_ptr<vko::RenderPass> m_renderPass;
        nstl::unique_ptr<vko::Image> m_depthImage;
        nstl::unique_ptr<vko::DeviceMemory> m_depthImageMemory;
        nstl::unique_ptr<vko::ImageView> m_depthImageView;
        std::size_t m_width = 0;
        std::size_t m_height = 0;

        nstl::vector<RendererFrameResources> m_frameResources;
        std::size_t m_nextFrameResourcesIndex = 0;

        BufferHandle m_cameraBuffer;
        TestCameraTransform m_cameraTransform;
        TestCameraParameters m_cameraParameters;
        TestLightParameters m_lightParameters;
        UniformConfiguration m_frameUniformConfiguration;
        DescriptorSetLayoutHandle m_frameDescriptorSetLayout;

        nstl::vector<TestObject> m_oneFrameTestObjects;
    };
}
