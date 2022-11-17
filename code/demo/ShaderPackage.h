#pragma once

#include "Hash.h"

#include "nstl/unordered_map.h"
#include "nstl/string.h"
#include "nstl/string_view.h"

// TODO store Mesh::Metadata and DescriptorSetConfiguration instead
struct ShaderConfiguration
{
    bool hasColor = false;
    bool hasTexCoord = false;
    bool hasNormal = false;
    bool hasTangent = false;
    bool hasTexture = false;
    bool hasNormalMap = false;

    bool operator==(ShaderConfiguration const&) const = default;
};

namespace nstl
{
    template<>
    struct hash<ShaderConfiguration>
    {
        std::size_t operator()(ShaderConfiguration const& rhs) const
        {
            std::size_t seed = 0;
            vkc::hash::combine(seed, rhs.hasColor);
            vkc::hash::combine(seed, rhs.hasTexCoord);
            vkc::hash::combine(seed, rhs.hasNormal);
            vkc::hash::combine(seed, rhs.hasTangent);
            vkc::hash::combine(seed, rhs.hasTexture);
            vkc::hash::combine(seed, rhs.hasNormalMap);
            return seed;
        }
    };
}

class ShaderPackage
{
public:
    ShaderPackage(nstl::string_view path);

    nstl::string const* get(ShaderConfiguration const& config) const;

    nstl::unordered_map<ShaderConfiguration, nstl::string> const& getAll() const { return m_shaders; }

private:
    nstl::unordered_map<ShaderConfiguration, nstl::string> m_shaders;
};
