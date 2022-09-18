#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "Handles.h"
#include "TestObject.h"

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
    struct RendererFrameResources;
    class ResourceManager;
    struct TestObject;

    class Renderer
    {
    public:
        Renderer(std::string const& name, bool enableValidationLayers, vko::Window const& window, std::function<void(vko::DebugMessage)> onDebugMessage = {});
        ~Renderer();

        ResourceManager& getResourceManager() const { return *m_resourceManager; }

        void addTestObject(TestObject object);
        void setCameraTransform(TestCameraTransform transform);

        void addOneFrameTestObject(TestObject object);

        void draw();

    private:
        void createCameraResources();
        void recordCommandBuffer(std::size_t imageIndex, RendererFrameResources& frameResources);

        void updateCameraBuffer();

    private:
        vko::Window const& m_window;

        // TODO don't use unique_ptrs

        std::unique_ptr<vkr::Application> m_application; // TODO don't use
        std::unique_ptr<ResourceManager> m_resourceManager;

        std::unique_ptr<vko::Swapchain> m_swapchain;
        std::vector<std::unique_ptr<vko::Framebuffer>> m_swapchainFramebuffers;
        std::vector<std::unique_ptr<vko::ImageView>> m_swapchainImageViews;
        std::unique_ptr<vko::RenderPass> m_renderPass;
        std::unique_ptr<vko::Image> m_depthImage;
        std::unique_ptr<vko::DeviceMemory> m_depthImageMemory;
        std::unique_ptr<vko::ImageView> m_depthImageView;

        std::vector<RendererFrameResources> m_frameResources;
        std::size_t m_nextFrameResourcesIndex = 0;

        BufferHandle m_cameraBuffer;
        TestCameraTransform m_cameraTransform;
        TestCameraParameters m_cameraParameters;
        TestLightParameters m_lightParameters;

        std::vector<TestObject> m_testObjects;
        std::vector<TestObject> m_oneFrameTestObjects;
    };
}
