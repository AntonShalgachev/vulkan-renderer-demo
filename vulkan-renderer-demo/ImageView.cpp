#include "ImageView.h"

#include "ServiceLocator.h"
#include "Renderer.h"

namespace vkr
{
	ImageView::ImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(temp::getDevice(), &imageViewCreateInfo, nullptr, &m_imageView) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture image view!");
	}

	ImageView::~ImageView()
	{
		vkDestroyImageView(temp::getDevice(), m_imageView, nullptr);
	}
}
