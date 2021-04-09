#include "VertexLayout.h"

#include <vulkan/vulkan.h>
#include <algorithm>
#include <stdexcept>
#include <iterator>

namespace
{
    VkFormat findAttributeFormat(vkr::VertexLayout::AttributeType type, vkr::VertexLayout::ComponentType componentType)
    {
        // TODO compe up with a more solid solution
        if (type == vkr::VertexLayout::AttributeType::Vec2 && componentType == vkr::VertexLayout::ComponentType::Float)
            return VK_FORMAT_R32G32_SFLOAT;
        if (type == vkr::VertexLayout::AttributeType::Vec3 && componentType == vkr::VertexLayout::ComponentType::Float)
            return VK_FORMAT_R32G32B32_SFLOAT;

        throw std::runtime_error("unknown attribute");
    }

    VkIndexType findComponentType(vkr::VertexLayout::ComponentType indexType)
    {
        switch (indexType)
        {
        case vkr::VertexLayout::ComponentType::UnsignedByte:
            return VK_INDEX_TYPE_UINT8_EXT;
        case vkr::VertexLayout::ComponentType::UnsignedShort:
            return VK_INDEX_TYPE_UINT16;
        case vkr::VertexLayout::ComponentType::UnsignedInt:
            return VK_INDEX_TYPE_UINT32;

        case vkr::VertexLayout::ComponentType::Byte:
        case vkr::VertexLayout::ComponentType::Short:
        case vkr::VertexLayout::ComponentType::Int:
        case vkr::VertexLayout::ComponentType::Float:
        case vkr::VertexLayout::ComponentType::Double:
            throw std::invalid_argument("Given component type isn't valid for indices");
        }

        throw std::invalid_argument("indexType");
    }
}

void vkr::VertexLayout::setBindings(std::vector<Binding> const& bindings)
{
    std::transform(bindings.begin(), bindings.end(), std::back_inserter(m_bindingOffsets),
                   [](VertexLayout::Binding const& binding) { return static_cast<VkDeviceSize>(binding.offset); });

    m_bindingDescriptions.reserve(bindings.size());

    for (std::size_t i = 0; i < bindings.size(); i++)
    {
        Binding const& binding = bindings[i];
        VkVertexInputBindingDescription& bindingDescription = m_bindingDescriptions.emplace_back();

        bindingDescription.binding = static_cast<uint32_t>(i);
        bindingDescription.stride = static_cast<uint32_t>(binding.stride);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        m_attributeDescriptions.reserve(m_attributeDescriptions.size() + binding.attributes.size());

        for (Attribute const& attribute : binding.attributes)
        {
            VkVertexInputAttributeDescription& attributeDescription = m_attributeDescriptions.emplace_back();

            attributeDescription.binding = static_cast<uint32_t>(i);
            attributeDescription.location = static_cast<uint32_t>(attribute.location);
            attributeDescription.format = ::findAttributeFormat(attribute.type, attribute.componentType);
            attributeDescription.offset = static_cast<uint32_t>(attribute.offset);
        }
    }
}

void vkr::VertexLayout::setIndexType(ComponentType indexType)
{
    m_indexType = ::findComponentType(indexType);
}
