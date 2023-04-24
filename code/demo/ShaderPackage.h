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

    size_t hash() const;
};

namespace nstl
{
    template<>
    struct hash<ShaderConfiguration>
    {
        size_t operator()(ShaderConfiguration const& rhs) const
        {
            return rhs.hash();
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
