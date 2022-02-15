#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vkr
{
	class VertexLayoutDescriptions
	{
		friend class VertexLayout;

	public:
		std::vector<VkVertexInputBindingDescription> const& getBindingDescriptions() const { return m_bindingDescriptions; }
		std::vector<VkVertexInputAttributeDescription> const& getAttributeDescriptions() const { return m_attributeDescriptions; };

		bool operator==(VertexLayoutDescriptions const& rhs) const;
		std::size_t computeHash() const;

	private:
		std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
	};
}

namespace std
{
	template<>
	struct hash<vkr::VertexLayoutDescriptions>
	{
		std::size_t operator()(vkr::VertexLayoutDescriptions const& rhs) const
		{
			return rhs.computeHash();
		}
	};
}
