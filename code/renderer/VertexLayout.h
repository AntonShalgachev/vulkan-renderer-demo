#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "VertexLayoutDescriptions.h"

namespace vkr
{
    // TODO rename to DataLayout or MeshDataLayout
    class VertexLayout
    {
    public:
        enum class AttributeType
        {
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4,
        };

        enum class ComponentType
        {
            Byte,
            UnsignedByte,
            Short,
            UnsignedShort,
            Int,
            UnsignedInt,
            Float,
            Double,
        };

        struct Attribute
        {
            Attribute(std::size_t location, AttributeType type, ComponentType componentType, std::size_t offset)
                : location(location)
                , type(type)
                , componentType(componentType)
                , offset(offset)
            {

            }

            std::size_t location = 0;
            AttributeType type = AttributeType::Vec2;
            ComponentType componentType = ComponentType::Float;
            std::size_t offset = 0;
        };

        struct Binding
        {
            Binding(std::size_t offset, std::size_t length, std::size_t stride) : offset(offset), length(length), stride(stride) {}

            template<class... Args>
            Binding& addAttribute(Args&&... args)
            {
                attributes.emplace_back(std::forward<Args>(args)...);
                return *this;
            }

            std::size_t offset = 0;
            std::size_t length = 0;
            std::size_t stride = 0;
            std::vector<Attribute> attributes;
        };

        VertexLayoutDescriptions const& getDescriptions() const { return m_descriptions; }

        void setBindings(std::vector<Binding> const& bindings);
        std::vector<VkDeviceSize> const& getBindingOffsets() const { return m_bindingOffsets; }

        void setIndexType(ComponentType indexType);
        VkIndexType getIndexType() const { return m_indexType; }

        void setIndexDataOffset(std::size_t offset) { m_indexDataOffset = offset; }
        VkDeviceSize getIndexDataOffset() const { return m_indexDataOffset; }

        void setIndexCount(std::size_t count) { m_indexCount = count; }
        VkDeviceSize getIndexCount() const { return m_indexCount; }

    private:
        VertexLayoutDescriptions m_descriptions;

        std::vector<VkDeviceSize> m_bindingOffsets;
        VkIndexType m_indexType = VK_INDEX_TYPE_UINT16;
        VkDeviceSize m_indexDataOffset = 0;
        VkDeviceSize m_indexCount = 0;
    };
}
