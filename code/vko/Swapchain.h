#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace vko
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
        VkExtent2D getExtent() const;
        VkSurfaceFormatKHR getSurfaceFormat() const;

        nstl::span<VkImage const> getRawImages() const { return m_rawImages; }

    private:
        void retrieveImages();

    private:
        Allocator m_allocator{ AllocatorScope::Swapchain };
        Device const& m_device;

        UniqueHandle<VkSwapchainKHR> m_handle;

        Config m_config;

        nstl::vector<VkImage> m_rawImages;
    };
}
