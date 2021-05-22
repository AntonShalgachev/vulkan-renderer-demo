#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vkr
{
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

        struct Descriptions
		{
            friend class VertexLayout;

        public:
			std::vector<VkVertexInputBindingDescription> const& getBindingDescriptions() const { return m_bindingDescriptions; }
			std::vector<VkVertexInputAttributeDescription> const& getAttributeDescriptions() const { return m_attributeDescriptions; };

        private:
			std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
        };

        Descriptions const& getDescriptions() const { return m_descriptions; }

        void setBindings(std::vector<Binding> const& bindings);
        std::vector<VkDeviceSize> const& getBindingOffsets() const { return m_bindingOffsets; }

        void setIndexType(ComponentType indexType);
        VkIndexType getIndexType() const { return m_indexType; }

        void setIndexDataOffset(std::size_t offset) { m_indexDataOffset = offset; }
        VkDeviceSize getIndexDataOffset() const { return m_indexDataOffset; }

    private:
        Descriptions m_descriptions;

        std::vector<VkDeviceSize> m_bindingOffsets;
        VkIndexType m_indexType = VK_INDEX_TYPE_UINT16;
        VkDeviceSize m_indexDataOffset = 0;
    };
}
