#pragma once

#include "vko/UniqueHandle.h"
#include "vko/Allocator.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Image;
	class Device;

	class ImageView
	{
	public:
		explicit ImageView(Device const& device, vko::Image const& image, VkImageAspectFlags aspectFlags);
		~ImageView();

        ImageView(ImageView const&) = default;
        ImageView(ImageView&&) = default;
        ImageView& operator=(ImageView const&) = default;
        ImageView& operator=(ImageView&&) = default;

        VkImageView getHandle() const { return m_handle; }

    private:
        Allocator m_allocator{ AllocatorScope::ImageView };
		VkDevice m_device = VK_NULL_HANDLE;
		UniqueHandle<VkImageView> m_handle;
	};
}

