#pragma once
#include <vector>

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

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

        enum class AttributeComponentType
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
            Attribute(std::size_t location, AttributeType type, AttributeComponentType componentType, std::size_t offset)
                : location(location)
                , type(type)
                , componentType(componentType)
                , offset(offset)
            {

            }

            std::size_t location;
            AttributeType type;
            AttributeComponentType componentType;
            std::size_t offset;
        };

        struct Binding
        {
            Binding(std::size_t stride) : stride(stride) {}

            template<class... Args>
            Binding& addAttribute(Args&&... args)
            {
                attributes.emplace_back(std::forward<Args>(args)...);
                return *this;
            }

            std::size_t stride;
            std::vector<Attribute> attributes;
        };

        VertexLayout() = default;
    	VertexLayout(std::vector<Binding> const& bindings);

        std::vector<VkVertexInputBindingDescription> const& getBindingDescriptions() const { return m_bindingDescriptions; }
        std::vector<VkVertexInputAttributeDescription> const& getAttributeDescriptions() const { return m_attributeDescriptions; };

    private:
        std::vector<VkVertexInputBindingDescription> m_bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    };
}
