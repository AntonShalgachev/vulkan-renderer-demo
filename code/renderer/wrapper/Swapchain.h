#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "UniqueHandle.h"

namespace vkr
{
    class Image;
    class RenderPass;
    class Device;
    class Surface;
    class QueueFamily;

    class Swapchain
    {
    public:
        struct Config
        {
            VkSurfaceFormatKHR surfaceFormat;
            VkPresentModeKHR presentMode;
            VkExtent2D extent;
            uint32_t minImageCount;
            VkSurfaceTransformFlagBitsKHR preTransform;
        };

    public:
        Swapchain(Device const& device, Surface const& surface, QueueFamily const& graphics, QueueFamily const& presentation, Config config);
        ~Swapchain();

        Swapchain(Swapchain const&) = default;
        Swapchain(Swapchain&&) = default;
        Swapchain& operator=(Swapchain const&) = default;
        Swapchain& operator=(Swapchain&&) = default;

        VkSwapchainKHR getHandle() const { return m_handle; }
        VkExtent2D getExtent() const { return m_config.extent; }
        VkSurfaceFormatKHR getSurfaceFormat() const { return m_config.surfaceFormat; }

        std::size_t getImageCount() const { return m_images.size(); }
        std::vector<std::unique_ptr<vkr::Image>> const& getImages() const { return m_images; }

    private:
        void retrieveImages();

    private:
        Device const& m_device;

        UniqueHandle<VkSwapchainKHR> m_handle;

//         VkExtent2D m_extent;
//         VkSurfaceFormatKHR m_surfaceFormat;
        Config m_config;

        std::vector<std::unique_ptr<vkr::Image>> m_images;
    };
}
