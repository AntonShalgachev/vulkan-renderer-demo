#include "ImageView.h"

#include "vko/Image.h"
#include "vko/Device.h"
#include "vko/Assert.h"

namespace vko
{
	ImageView::ImageView(Device const& device, vko::Image const& image, VkImageAspectFlags aspectFlags) : m_device(device.getHandle())
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = image.getHandle();
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = image.getFormat();
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		VKO_VERIFY(vkCreateImageView(m_device, &imageViewCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
	}

	ImageView::~ImageView()
	{
		if (m_handle)
			vkDestroyImageView(m_device, m_handle, &m_allocator.getCallbacks());
	}
}
