#include "VertexLayoutDescriptions.h"
#include "Hash.h"

namespace
{
	auto tie(VkVertexInputBindingDescription const& desc)
	{
		return std::tie(desc.binding, desc.stride, desc.inputRate);
	}

	auto tie(VkVertexInputAttributeDescription const& desc)
	{
		return std::tie(desc.location, desc.binding, desc.format, desc.offset);
	}

	template<typename T>
	bool descEqual(T const& lhs, T const& rhs)
	{
		return ::tie(lhs) == ::tie(rhs);
	}

	template<typename T>
	bool equal(std::vector<T> const& lhs, std::vector<T> const& rhs)
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), ::descEqual<T>);
	}
}

bool vkr::VertexLayoutDescriptions::operator==(VertexLayoutDescriptions const& rhs) const
{
	return ::equal(m_bindingDescriptions, rhs.m_bindingDescriptions) && ::equal(m_attributeDescriptions, rhs.m_attributeDescriptions);
}

std::size_t vkr::VertexLayoutDescriptions::computeHash() const
{
	std::size_t seed = 0;
	vkr::hash::combine(seed, m_bindingDescriptions);
	vkr::hash::combine(seed, m_attributeDescriptions);
	return seed;
}
