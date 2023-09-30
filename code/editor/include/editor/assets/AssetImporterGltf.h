#pragma once

#include "nstl/string_view.h"
#include "nstl/vector.h"

struct cgltf_data;

namespace editor::assets
{
    class AssetDatabase;
    struct Uuid;
    struct ImportDescription;

    class AssetImporterGltf
    {
    public:
        AssetImporterGltf(AssetDatabase& database);

        nstl::vector<Uuid> importAsset(ImportDescription const& desc) const;

    private:
        nstl::vector<Uuid> parseGltfData(cgltf_data const& data, ImportDescription const& desc) const;

        AssetDatabase& m_database;
    };
}
