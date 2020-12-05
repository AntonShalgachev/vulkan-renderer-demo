#pragma once

#include "framework.h"

namespace vkr
{
	class ImageView
	{
	public:
		ImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		~ImageView();

        VkImageView getHandle() const { return m_imageView; }

	private:
		VkImageView m_imageView;
	};
}

