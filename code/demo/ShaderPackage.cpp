#include "ShaderPackage.h"

#include "common/Utils.h"
#include "common/json-nstl.h"

#include "yyjsoncpp/yyjsoncpp.h"

ShaderPackage::ShaderPackage(nstl::string_view path)
{
    namespace json = yyjsoncpp;

    auto packageMetadataPath = nstl::string{ path } + "/package.json";

    auto contents = vkc::utils::readTextFile(packageMetadataPath);

    json::doc doc;
    if (!doc.read(contents.data(), contents.size()))
        assert(false);

    json::value_ref root = doc.get_root();

    for (json::value_ref variant : root["variants"].get_array())
    {
        ShaderConfiguration configuration;

        for (json::pair const& p : variant["configuration"].get_object())
        {
            nstl::string_view key = p.key.get<nstl::string_view>();
            nstl::string_view value = p.value.get<nstl::string_view>();

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

        nstl::string_view variantPath = variant["path"].get<nstl::string_view>();

        m_shaders.insert_or_assign(configuration, nstl::string{ path } + "/" + variantPath);
    }
}

nstl::string const* ShaderPackage::get(ShaderConfiguration const& config) const
{
    auto it = m_shaders.find(config);
    if (it == m_shaders.end())
        return nullptr;

    return &it->value();
}

size_t ShaderConfiguration::hash() const
{
    return nstl::hash_values(hasColor, hasTexCoord, hasNormal, hasTangent, hasTangent, hasTexture, hasNormalMap);
}
