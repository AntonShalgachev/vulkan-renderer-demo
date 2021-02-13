#include "VertexLayout.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace
{
    VkFormat findAttributeFormat(vkr::VertexLayout::AttributeType type, vkr::VertexLayout::AttributeComponentType componentType)
    {
        // TODO compe up with a more solid solution
        if (type == vkr::VertexLayout::AttributeType::Vec2 && componentType == vkr::VertexLayout::AttributeComponentType::Float)
            return VK_FORMAT_R32G32_SFLOAT;
        if (type == vkr::VertexLayout::AttributeType::Vec3 && componentType == vkr::VertexLayout::AttributeComponentType::Float)
            return VK_FORMAT_R32G32B32_SFLOAT;

        throw std::runtime_error("unknown attribute");
    }
}

vkr::VertexLayout::VertexLayout(std::vector<Binding> const& bindings)
{
    m_bindingDescriptions.reserve(bindings.size());

    for (std::size_t i = 0; i < bindings.size(); i++)
    {
        Binding const& binding = bindings[i];
        VkVertexInputBindingDescription& bindingDescription = m_bindingDescriptions.emplace_back();

        bindingDescription.binding = static_cast<uint32_t>(i);
        bindingDescription.stride = static_cast<uint32_t>(binding.stride);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        for (Attribute const& attribute : binding.attributes)
        {
            VkVertexInputAttributeDescription& attributeDescription = m_attributeDescriptions.emplace_back();

            attributeDescription.binding = static_cast<uint32_t>(i);
            attributeDescription.location = static_cast<uint32_t>(attribute.location);
            attributeDescription.format = findAttributeFormat(attribute.type, attribute.componentType);
            attributeDescription.offset = static_cast<uint32_t>(attribute.offset);
        }
    }
}
