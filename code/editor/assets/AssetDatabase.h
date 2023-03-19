#pragma once

#include "Uuid.h"

#include "nstl/vector.h"
#include "nstl/string_view.h"
#include "nstl/unique_ptr.h"
#include "nstl/string.h"
#include "nstl/unordered_map.h"
#include "nstl/optional.h"

namespace editor::assets
{
    class AssetImporterGltf;
    class AssetImporterImage;
    struct ImportDescription;

    enum class AssetType
    {
        Image,
        Material,
        Mesh,
        Scene,
    };

    struct AssetMetadata
    {
        nstl::string name;
        AssetType type = AssetType::Image;
        nstl::vector<nstl::string> files;
    };

    class AssetDatabase
    {
    public:
        AssetDatabase();

        nstl::vector<Uuid> importAsset(nstl::string_view path);
        nstl::vector<Uuid> importAsset(ImportDescription const& desc);

        Uuid createAsset(AssetType type, nstl::string_view name);
        void addAssetFile(Uuid id, nstl::span<unsigned char const> bytes, nstl::string_view filename);
        void addAssetFile(Uuid id, nstl::string_view bytes, nstl::string_view filename);

    private:
        nstl::optional<AssetMetadata> loadMetadataFile(Uuid id);
        void saveMetadataFile(Uuid id, AssetMetadata const& metadata);

    private:
        nstl::unique_ptr<AssetImporterGltf> m_assetImporterGltf;
        nstl::unique_ptr<AssetImporterImage> m_assetImporterImage;
    };
}
