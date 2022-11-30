#pragma once

#include "nstl/unordered_map.h"
#include "nstl/string.h"
#include "nstl/string_view.h"
#include "nstl/hash.h"

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
            nstl::hash_combine(seed, rhs.hasColor);
            nstl::hash_combine(seed, rhs.hasTexCoord);
            nstl::hash_combine(seed, rhs.hasNormal);
            nstl::hash_combine(seed, rhs.hasTangent);
            nstl::hash_combine(seed, rhs.hasTexture);
            nstl::hash_combine(seed, rhs.hasNormalMap);
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
