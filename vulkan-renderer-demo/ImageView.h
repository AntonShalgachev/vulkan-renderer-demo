#pragma once

#include "framework.h"

namespace vkr
{
    class Image;

	class ImageView
	{
	public:
		explicit ImageView(vkr::Image const& image, VkImageAspectFlags aspectFlags);
		~ImageView();

        ImageView(ImageView const&) = delete;
        ImageView(ImageView&&) = delete;
        ImageView& operator=(ImageView const&) = delete;
        ImageView& operator=(ImageView&&) = delete;

        VkImageView getHandle() const { return m_handle; }

	private:
		VkImageView m_handle = VK_NULL_HANDLE;
	};
}

