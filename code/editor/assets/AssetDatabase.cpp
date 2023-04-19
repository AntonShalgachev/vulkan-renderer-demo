#include "AssetDatabase.h"

#include "ImportDescription.h"

#include "AssetImporterGltf.h"
#include "AssetImporterImage.h"
#include "AssetData.h"

#include "common/Utils.h"
#include "common/json-nstl.h"
#include "common/json-tiny-ctti.h"
#include "common/json-glm.h"

#include "fs/directory.h"
#include "fs/file.h"
#include "path/path.h"
#include "yyjsoncpp/yyjsoncpp.h"
#include "logging/logging.h"

namespace
{
    nstl::string_view assetsRoot = "data/assets";

    uint16_t assetMetadataVersion = 1;

    // TODO merge these 2 functions
    // TODO find a better name? It constructs a path and creates directories
    nstl::string constructAndCreateAssetPath(editor::assets::Uuid id, nstl::string_view filename)
    {
        nstl::string rootPath = path::join(assetsRoot, id.toString());
        fs::create_directories(rootPath);

        return path::join(rootPath, filename);
    }

    nstl::string constructAssetPath(editor::assets::Uuid id, nstl::string_view filename)
    {
        return path::join(assetsRoot, id.toString(), filename);
    }
}

editor::assets::AssetDatabase::AssetDatabase()
{
    m_assetImporterGltf = nstl::make_unique<AssetImporterGltf>(*this);
    m_assetImporterImage = nstl::make_unique<AssetImporterImage>(*this);
}

editor::assets::AssetDatabase::~AssetDatabase() = default;

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
    logging::info("Importing {} ({})", desc.name, desc.extension);

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
        .version = assetMetadataVersion,
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
    AssetMetadata metadata = getMetadata(id);
    metadata.files.push_back(filename);
    saveMetadataFile(id, metadata);
}

void editor::assets::AssetDatabase::addAssetFile(Uuid id, nstl::string_view bytes, nstl::string_view filename)
{
    char const* data = bytes.data();
    size_t size = bytes.size();

    // TODO don't use reinterpret_cast
    return addAssetFile(id, { reinterpret_cast<unsigned char const*>(data), size }, filename);
}

void editor::assets::AssetDatabase::addAssetFile(Uuid id, nstl::blob const& bytes, nstl::string_view filename)
{
    return addAssetFile(id, static_cast<nstl::span<unsigned char const>>(bytes), filename);
}

editor::assets::AssetMetadata editor::assets::AssetDatabase::getMetadata(Uuid id) const
{
    nstl::optional<AssetMetadata> metadata = loadMetadataFile(id);

    assert(metadata);
    assert(metadata->version == assetMetadataVersion);

    return *metadata;
}

nstl::optional<editor::assets::AssetMetadata> editor::assets::AssetDatabase::loadMetadataFile(Uuid id) const
{
    namespace json = yyjsoncpp;

    nstl::string path = constructAndCreateAssetPath(id, "asset.json");

    fs::file f{ path, fs::open_mode::read };
    nstl::blob content{ f.size() };
    f.read(content.data(), content.size());
    f.close();

    json::doc doc;
    if (!doc.read(content.cdata(), content.size()))
        return {};

    return doc.get_root().get<AssetMetadata>();
}

void editor::assets::AssetDatabase::saveMetadataFile(Uuid id, AssetMetadata const& metadata) const
{
    assert(metadata.version == assetMetadataVersion);

    namespace json = yyjsoncpp;

    json::mutable_doc doc;
    json::mutable_value_ref root = doc.create_value(metadata);
    doc.set_root(root);

    nstl::string result = doc.write(json::write_flags::pretty);
    nstl::span<unsigned char const> resultSpan = { reinterpret_cast<unsigned char const*>(result.data()), result.length() }; // TODO fix it somehow

    nstl::string path = constructAndCreateAssetPath(id, "asset.json");

    fs::file f{ path, fs::open_mode::write };
    f.write(resultSpan.data(), resultSpan.size());
}

editor::assets::SceneData editor::assets::AssetDatabase::loadScene(Uuid id) const
{
    auto metadata = loadMetadataFile(id);
    assert(metadata);

    assert(metadata->type == AssetType::Scene);

    assert(metadata->files.size() == 1);

    nstl::string path = constructAssetPath(id, metadata->files[0]);

    fs::file f{ path, fs::open_mode::read };

    nstl::string contents;
    contents.resize(f.size());
    f.read(contents.data(), contents.size());

    namespace json = yyjsoncpp;

    json::doc doc;
    if (!doc.read(contents.data(), contents.size()))
        assert(false);

    json::value_ref root = doc.get_root();
    return root.get<SceneData>();
}

editor::assets::MeshData editor::assets::AssetDatabase::loadMesh(Uuid id) const
{
    assert(false); // TODO implement
    return {};
}

editor::assets::MaterialData editor::assets::AssetDatabase::loadMaterial(Uuid id) const
{
    assert(false); // TODO implement
    return {};
}
