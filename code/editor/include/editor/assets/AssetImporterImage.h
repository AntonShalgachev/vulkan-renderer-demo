#pragma once

#include "nstl/string_view.h"
#include "nstl/vector.h"

namespace editor::assets
{
    class AssetDatabase;
    struct Uuid;
    struct ImportDescription;

    class AssetImporterImage
    {
    public:
        AssetImporterImage(AssetDatabase& database);

        nstl::vector<Uuid> importAsset(ImportDescription const& desc) const;

    private:
        AssetDatabase& m_database;
    };
}
