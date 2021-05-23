#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vkr
{
	class VertexLayoutDescriptions
	{
		friend class VertexLayout;

	private:
		VertexLayoutDescriptions() = default;

	public:
		std::vector<VkVertexInputBindingDescription> const& getBindingDescriptions() const { return m_bindingDescriptions; }
		std::vector<VkVertexInputAttributeDescription> const& getAttributeDescriptions() const { return m_attributeDescriptions; };

		bool operator==(VertexLayoutDescriptions const& rhs) const;

	private:
		std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
	};
}
