FetchContent_Declare(
    cgltf
    URL https://github.com/jkuhlmann/cgltf/archive/refs/tags/v1.13.tar.gz # 1.13
)
FetchDependency(cgltf)

add_library(cgltf
    ${cgltf_SOURCE_DIR}/cgltf.h
    cgltf.c
)
add_library(cgltf::cgltf ALIAS cgltf)

target_include_directories(cgltf PUBLIC
    "${cgltf_SOURCE_DIR}"
)
