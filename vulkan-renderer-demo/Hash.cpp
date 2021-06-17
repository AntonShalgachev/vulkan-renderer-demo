#include "Hash.h"
#include <vulkan/vulkan.h>

std::size_t std::hash<VkExtent2D>::operator()(VkExtent2D const& rhs) const
{
	std::size_t seed = 0;
	vkr::hash::combine(seed, rhs.width);
	vkr::hash::combine(seed, rhs.height);
	return seed;
}

std::size_t std::hash<VkVertexInputBindingDescription>::operator()(VkVertexInputBindingDescription const& rhs) const
{
	std::size_t seed = 0;
	vkr::hash::combine(seed, rhs.binding);
	vkr::hash::combine(seed, rhs.stride);
	vkr::hash::combine(seed, rhs.inputRate);
	return seed;
}

std::size_t std::hash<VkVertexInputAttributeDescription>::operator()(VkVertexInputAttributeDescription const& rhs) const
{
	std::size_t seed = 0;
	vkr::hash::combine(seed, rhs.location);
	vkr::hash::combine(seed, rhs.binding);
	vkr::hash::combine(seed, rhs.format);
	vkr::hash::combine(seed, rhs.offset);
	return seed;
}
