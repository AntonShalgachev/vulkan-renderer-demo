#pragma once

#include "framework.h"

namespace vkr
{
    class Image;

	class ImageView
	{
	public:
		ImageView(vkr::Image const& image, VkImageAspectFlags aspectFlags);
		~ImageView();

        VkImageView getHandle() const { return m_imageView; }

	private:
		VkImageView m_imageView;
	};
}

