add_library(vkgfx
    "pch.h"

    "Buffer.h"
    "Buffer.cpp"
    "BufferMetadata.h"
    "Image.h"
    "Image.cpp"
    "ImageMetadata.h"
    "Texture.h"
    "Material.h"
    "Mesh.h"
    "PipelineKey.h"
    "PipelineKey.cpp"
    "Handles.h"
    "ResourceManager.h"
    "ResourceManager.cpp"
    "Renderer.h"
    "Renderer.cpp"

    "ResourceContainer.h"
    "ResourceContainer.cpp"
    "TestObject.h"
)

target_include_directories(vkgfx PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_features(vkgfx PUBLIC cxx_std_20)

if(DEMO_PCH)
    target_precompile_headers(vkgfx PRIVATE "pch.h")
endif()

target_link_libraries(vkgfx
    vko
    common
    vkr # TODO remove
)
