#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <vector>
#include <memory>
#include "UniqueHandle.h"

namespace vkr
{
    class Image;
    class RenderPass;

    class Swapchain : public Object
    {
    public:
        Swapchain(Application const& app);
        ~Swapchain();

        Swapchain(Swapchain const&) = default;
        Swapchain(Swapchain&&) = default;
        Swapchain& operator=(Swapchain const&) = default;
        Swapchain& operator=(Swapchain&&) = default;

        VkSwapchainKHR getHandle() const { return m_handle; }
        VkExtent2D getExtent() const { return m_extent; }
        VkSurfaceFormatKHR getSurfaceFormat() const { return m_surfaceFormat; }

        std::size_t getImageCount() const { return m_images.size(); }
        std::vector<std::unique_ptr<vkr::Image>> const& getImages() const { return m_images; }

    private:
        void retrieveImages();

    private:
        UniqueHandle<VkSwapchainKHR> m_handle;

        VkExtent2D m_extent;
        VkSurfaceFormatKHR m_surfaceFormat;

        std::vector<std::unique_ptr<vkr::Image>> m_images;
    };
}
