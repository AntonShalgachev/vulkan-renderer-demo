#pragma once

#include "framework.h"

namespace vkr
{
    class Image;
    class ImageView;
    class RenderPass;
    class Framebuffer;

    class Swapchain
    {
    public:
        Swapchain();
        ~Swapchain();

        void createFramebuffers(vkr::RenderPass const& renderPass, vkr::ImageView const& depthImageView);

        VkSwapchainKHR getHandle() const { return m_handle; }
        VkExtent2D getExtent() const { return m_extent; }
        VkSurfaceFormatKHR getSurfaceFormat() const { return m_surfaceFormat; }

        std::size_t getImageCount() const { return m_images.size(); }
        std::vector<std::unique_ptr<vkr::Image>> const& getImages() const { return m_images; }
        std::vector<std::unique_ptr<vkr::ImageView>> const& getImageViews() const { return m_imageViews; }
        std::vector<std::unique_ptr<vkr::Framebuffer>> const& getFramebuffers() const { return m_framebuffers; }

    private:
        void createSwapchain();
        void retrieveImages();
        void createImageViews();

    private:
        VkSwapchainKHR m_handle;

        VkExtent2D m_extent;
        VkSurfaceFormatKHR m_surfaceFormat;

        //std::vector<VkImage> m_images;
        std::vector<std::unique_ptr<vkr::Image>> m_images;
        std::vector<std::unique_ptr<vkr::ImageView>> m_imageViews;
        std::vector<std::unique_ptr<vkr::Framebuffer>> m_framebuffers;
    };
}