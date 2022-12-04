FetchContent_Declare(
    dds-ktx
    URL https://github.com/septag/dds-ktx/archive/refs/tags/v1.1.tar.gz # 1.1
)

FetchDependency(dds-ktx)

add_library(dds-ktx
    ${dds-ktx_SOURCE_DIR}/dds-ktx.h
    dds-ktx.c
)
add_library(dds-ktx::dds-ktx ALIAS dds-ktx)

target_include_directories(dds-ktx PUBLIC
    "${dds-ktx_SOURCE_DIR}"
)
