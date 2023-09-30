add_library(editor_assets
    "AssetDatabase.h"
    "AssetDatabase.cpp"
    "AssetImporterGltf.h"
    "AssetImporterGltf.cpp"
    "AssetImporterImage.h"
    "AssetImporterImage.cpp"
    
    "Uuid.h"
    "Uuid.cpp"

    "ImportDescription.h"
    "AssetData.h"
)
add_library(editor::assets ALIAS editor_assets)

demo_set_common_properties(editor_assets)

target_link_libraries(editor_assets
    nstl
    memory
    cgltf::cgltf
    logging
    fs
    path
    stb
    Vulkan::Vulkan
    yyjsoncpp
    platform # TODO remove (only needed for Uuid)
    tglm
    tiny_ktx
)
