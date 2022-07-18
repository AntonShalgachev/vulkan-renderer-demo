#pragma once

#include "Hash.h"

#include <filesystem>
#include <unordered_map>

namespace vkr
{
    struct ShaderConfiguration
    {
        bool hasColor = false;
        bool hasTexCoord = false;
        bool hasNormal = false;
        bool hasTangent = false;
        bool hasTexture = false;

        bool operator==(ShaderConfiguration const&) const = default;
    };
}

namespace std
{
    template<>
    struct hash<vkr::ShaderConfiguration>
    {
        std::size_t operator()(vkr::ShaderConfiguration const& rhs) const
        {
            std::size_t seed = 0;
            vkr::hash::combine(seed, rhs.hasColor);
            vkr::hash::combine(seed, rhs.hasTexCoord);
            vkr::hash::combine(seed, rhs.hasNormal);
            vkr::hash::combine(seed, rhs.hasTangent);
            vkr::hash::combine(seed, rhs.hasTexture);
            return seed;
        }
    };
}

namespace vkr
{
    class ShaderPackage
    {
    public:
        ShaderPackage(std::filesystem::path path);

        std::string const* get( ShaderConfiguration const& config) const;

    private:
        std::unordered_map<ShaderConfiguration, std::string> m_shaders;
    };
}
