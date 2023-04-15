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

target_compile_features(editor_assets PUBLIC cxx_std_20)

target_link_libraries(editor_assets
    nstl
    memory
    cgltf::cgltf
    logging
    fs
    path
    stb
    ktx
    Vulkan::Vulkan
    yyjsoncpp
    platform # TODO remove (only needed for Uuid)
)
