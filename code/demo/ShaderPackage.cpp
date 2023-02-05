#include "ShaderPackage.h"

#include "common/Utils.h"

#include "yyjson.h"

ShaderPackage::ShaderPackage(nstl::string_view path)
{
    auto packageMetadataPath = nstl::string{ path } + "/package.json";

    {
        // TODO make a nice wrapper around yyjson

        yyjson_doc* doc = nullptr;

        {
            auto contents = vkc::utils::readTextFile(packageMetadataPath);

            yyjson_read_err error{};
            doc = yyjson_read_opts(contents.data(), contents.size(), 0, nullptr, &error);
        }

        yyjson_val* jRoot = yyjson_doc_get_root(doc);

        yyjson_val* jVariants = yyjson_obj_get(jRoot, "variants");
        size_t idx, max;
        yyjson_val* jVariant;
        yyjson_arr_foreach(jVariants, idx, max, jVariant)
        {
            ShaderConfiguration configuration;

            yyjson_val* jConfiguration = yyjson_obj_get(jVariant, "configuration");

            yyjson_val* jKey = nullptr;
            yyjson_val* jValue = nullptr;
            yyjson_obj_iter iter;
            yyjson_obj_iter_init(jConfiguration, &iter);
            while ((jKey = yyjson_obj_iter_next(&iter)))
            {
                jValue = yyjson_obj_iter_get_val(jKey);

                nstl::string_view key = yyjson_get_str(jKey);
                nstl::string_view value = yyjson_get_str(jValue);

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

            yyjson_val* jPath = yyjson_obj_get(jVariant, "path");

            m_shaders.insert_or_assign(configuration, nstl::string{ path } + "/" + nstl::string{ yyjson_get_str(jPath) });
        }

        yyjson_doc_free(doc);
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
