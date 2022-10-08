#include "ShaderPackage.h"

#include "nlohmann/json.hpp"

#include <fstream>

ShaderPackage::ShaderPackage(std::string_view path)
{
    auto packageMetadataPath = std::string{ path } + "/package.json";

    nlohmann::json j;

    {
        std::ifstream ifs{ packageMetadataPath };
        if (!ifs.is_open())
            throw std::runtime_error("Failed to open shader package");
        ifs >> j;
    }

    for (auto const& variant : j["variants"])
    {
        ShaderConfiguration configuration;
        for (auto const& [key, value] : variant["configuration"].items())
        {
            if (key == "HAS_VERTEX_COLOR")
                configuration.hasColor = (value == "");
            if (key == "HAS_TEX_COORD")
                configuration.hasTexCoord = (value == "");
            if (key == "HAS_NORMAL")
                configuration.hasNormal = (value == "");
            if (key == "HAS_TANGENT")
                configuration.hasTangent = (value == "");
            if (key == "HAS_TEXTURE")
                configuration.hasTexture = (value == "");
            if (key == "HAS_NORMAL_MAP")
                configuration.hasNormalMap = (value == "");
        }

        m_shaders[configuration] = std::string{ path } + "/" + variant["path"].get<std::string>();
    }
}

std::string const* ShaderPackage::get(ShaderConfiguration const& config) const
{
    auto it = m_shaders.find(config);
    if (it == m_shaders.end())
        return nullptr;

    return &it->second;
}
