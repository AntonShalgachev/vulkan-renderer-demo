find_package(Vulkan REQUIRED)

add_library(editor
    "include/editor/assets/AssetDatabase.h"
    "include/editor/assets/AssetImporterGltf.h"
    "include/editor/assets/AssetImporterImage.h"
    "include/editor/assets/Uuid.h"
    "include/editor/assets/ImportDescription.h"
    "include/editor/assets/AssetData.h"

    "src/assets/AssetDatabase.cpp"
    "src/assets/AssetImporterGltf.cpp"
    "src/assets/AssetImporterImage.cpp"
    "src/assets/Uuid.cpp"
)

target_include_directories(editor PUBLIC "include")

demo_set_common_properties(editor)

target_mark_includes_system(editor cgltf::cgltf)

target_link_libraries(editor
    nstl
    memory
    cgltf::cgltf
    logging
    fs
    path
    stb
    Vulkan::Vulkan # TODO remove (needed for ktx2)
    yyjsoncpp
    platform # TODO remove (only needed for Uuid)
    tglm
    tiny_ktx
)
