#pragma once

#include <vulkan/vulkan.h>
#include "UniqueHandle.h"

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
		Device const& m_device;
		UniqueHandle<VkImageView> m_handle;
	};
}

