#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include "UniqueHandle.h"

namespace vkr
{
    class Image;

	class ImageView : public Object
	{
	public:
		explicit ImageView(Application const& app, vkr::Image const& image, VkImageAspectFlags aspectFlags);
		~ImageView();

        ImageView(ImageView const&) = default;
        ImageView(ImageView&&) = default;
        ImageView& operator=(ImageView const&) = default;
        ImageView& operator=(ImageView&&) = default;

        VkImageView getHandle() const { return m_handle; }

	private:
		UniqueHandle<VkImageView> m_handle;
	};
}

