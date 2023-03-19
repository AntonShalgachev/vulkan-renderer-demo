#include "AssetDatabase.h"

#include "ImportDescription.h"

#include "AssetImporterGltf.h"
#include "AssetImporterImage.h"

#include "common/Utils.h"
#include "common/json-nstl.h"

#include "fs/directory.h"
#include "fs/file.h"
#include "path/path.h"

#include "yyjsoncpp/yyjsoncpp.h"

namespace
{
    nstl::string_view assetsRoot = "data/assets";

    // TODO find a better name? It constructs a path and creates directories
    nstl::string constructAndCreateAssetPath(editor::assets::Uuid id, nstl::string_view filename)
    {
        nstl::string rootPath = path::join(assetsRoot, id.toString());
        fs::create_directories(rootPath);

        return path::join(rootPath, filename);
    }
}

namespace yyjsoncpp
{
    template<>
    struct serializer<editor::assets::AssetType>
    {
        static optional<editor::assets::AssetType> from_json(value_ref value)
        {
            if (value.get_type() != type::string)
                return {};

            nstl::string_view str = value.get_string();

            if (str == "image") return editor::assets::AssetType::Image;
            if (str == "material") return editor::assets::AssetType::Material;
            if (str == "mesh") return editor::assets::AssetType::Mesh;
            if (str == "scene") return editor::assets::AssetType::Scene;

            return {};
        }

        static mutable_value_ref to_json(mutable_doc& doc, editor::assets::AssetType const& type)
        {
            switch (type)
            {
            case editor::assets::AssetType::Image: return doc.create_string("image");
            case editor::assets::AssetType::Material: return doc.create_string("material");
            case editor::assets::AssetType::Mesh: return doc.create_string("mesh");
            case editor::assets::AssetType::Scene: return doc.create_string("scene");
            }

            assert(false);
            return doc.create_null();
        }
    };
}

editor::assets::AssetDatabase::AssetDatabase()
{
    m_assetImporterGltf = nstl::make_unique<AssetImporterGltf>(*this);
    m_assetImporterImage = nstl::make_unique<AssetImporterImage>(*this);
}

nstl::vector<editor::assets::Uuid> editor::assets::AssetDatabase::importAsset(nstl::string_view path)
{
    if (path.empty())
        return {};

    fs::file f{ path, fs::open_mode::read };
    nstl::blob content{ f.size() };
    f.read(content.data(), content.size());
    f.close();

    path::parts parts = path::split_into_parts(path);

    ImportDescription desc = {
        .content = content,
        .parentDirectory = parts.parent_path,
        .name = parts.name_without_extension,
        .extension = parts.extension,
    };

    return importAsset(desc);
}

nstl::vector<editor::assets::Uuid> editor::assets::AssetDatabase::importAsset(ImportDescription const& desc)
{
    // TODO implement properly
    // TODO make sure uppercase extension also works

    if (desc.extension == "gltf")
        return m_assetImporterGltf->importAsset(desc);

    if (desc.extension == "jpg" || desc.extension == "jpeg" || desc.extension == "png")
        return m_assetImporterImage->importAsset(desc);

    assert(false);
    return {};
}

editor::assets::Uuid editor::assets::AssetDatabase::createAsset(AssetType type, nstl::string_view name)
{
    Uuid id = Uuid::generate();

    AssetMetadata metadata = {
        .name = name,
        .type = type,
        .files = {},
    };

    saveMetadataFile(id, metadata);

    return id;
}

void editor::assets::AssetDatabase::addAssetFile(Uuid id, nstl::span<unsigned char const> bytes, nstl::string_view filename)
{
    nstl::string filePath = constructAndCreateAssetPath(id, filename);

    {
        fs::file f{ filePath, fs::open_mode::write };
        f.write(bytes.data(), bytes.size());
    }
    
    // TODO avoid loading/saving metadata file for every asset modification
    nstl::optional<AssetMetadata> metadata = loadMetadataFile(id);
    assert(metadata);
    metadata->files.push_back(filename);
    saveMetadataFile(id, *metadata);
}

void editor::assets::AssetDatabase::addAssetFile(Uuid id, nstl::string_view bytes, nstl::string_view filename)
{
    char const* data = bytes.data();
    size_t size = bytes.size();

    // TODO don't use reinterpret_cast
    return addAssetFile(id, { reinterpret_cast<unsigned char const*>(data), size }, filename);
}

nstl::optional<editor::assets::AssetMetadata> editor::assets::AssetDatabase::loadMetadataFile(Uuid id)
{
    namespace json = yyjsoncpp;

    AssetMetadata metadata;

    nstl::string path = constructAndCreateAssetPath(id, "asset.json");

    fs::file f{ path, fs::open_mode::read };
    nstl::blob content{ f.size() };
    f.read(content.data(), content.size());
    f.close();

    json::doc doc;
    if (!doc.read(content.cdata(), content.size()))
        return {};

    json::value_ref root = doc.get_root();
    metadata.name = root["name"].get<nstl::string>();
    metadata.type = root["type"].get<AssetType>();
    metadata.files = root["files"].get<nstl::vector<nstl::string>>();

    return metadata;
}

void editor::assets::AssetDatabase::saveMetadataFile(Uuid id, AssetMetadata const& metadata)
{
    namespace json = yyjsoncpp;

    json::mutable_doc doc;
    json::mutable_object_ref root = doc.create_object();
    doc.set_root(root);

    root["name"] = metadata.name;
    root["type"] = metadata.type;
    root["files"] = metadata.files;

    nstl::string result = doc.write(json::write_flags::pretty);
    nstl::span<unsigned char const> resultSpan = { reinterpret_cast<unsigned char const*>(result.data()), result.length() }; // TODO fix it somehow

    nstl::string path = constructAndCreateAssetPath(id, "asset.json");

    fs::file f{ path, fs::open_mode::write };
    f.write(resultSpan.data(), resultSpan.size());
}
