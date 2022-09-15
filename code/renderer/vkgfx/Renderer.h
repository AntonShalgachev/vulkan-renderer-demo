#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>

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

        void draw();

    private:
        void recordCommandBuffer(std::size_t imageIndex, RendererFrameResources& frameResources);

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

        std::vector<TestObject> m_testObjects;
    };
}
