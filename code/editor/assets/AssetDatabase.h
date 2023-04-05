#pragma once

#include "Uuid.h"

#include "common/tiny_ctti.h"

#include "nstl/vector.h"
#include "nstl/string_view.h"
#include "nstl/unique_ptr.h"
#include "nstl/string.h"
#include "nstl/unordered_map.h"
#include "nstl/optional.h"

namespace nstl
{
    class blob;
}

namespace editor::assets
{
    enum class AssetType
    {
        Image,
        Material,
        Mesh,
        Scene,
    };
    TINY_CTTI_DESCRIBE_ENUM(AssetType, Image, Material, Mesh, Scene);

    struct AssetMetadata
    {
        uint16_t version = 0;
        nstl::string name;
        AssetType type = AssetType::Image;
        nstl::vector<nstl::string> files;
    };
    TINY_CTTI_DESCRIBE_STRUCT(AssetMetadata, version, name, type, files);
}

namespace editor::assets
{
    class AssetImporterGltf;
    class AssetImporterImage;
    struct ImportDescription;

    class AssetDatabase
    {
    public:
        AssetDatabase();
        ~AssetDatabase();

        nstl::vector<Uuid> importAsset(nstl::string_view path);
        nstl::vector<Uuid> importAsset(ImportDescription const& desc);

        Uuid createAsset(AssetType type, nstl::string_view name);
        void addAssetFile(Uuid id, nstl::span<unsigned char const> bytes, nstl::string_view filename);
        void addAssetFile(Uuid id, nstl::string_view bytes, nstl::string_view filename);
        void addAssetFile(Uuid id, nstl::blob const& bytes, nstl::string_view filename);

    private:
        AssetMetadata getMetadata(Uuid id) const;

        nstl::optional<AssetMetadata> loadMetadataFile(Uuid id) const;
        void saveMetadataFile(Uuid id, AssetMetadata const& metadata) const;

    private:
        nstl::unique_ptr<AssetImporterGltf> m_assetImporterGltf;
        nstl::unique_ptr<AssetImporterImage> m_assetImporterImage;
    };
}
