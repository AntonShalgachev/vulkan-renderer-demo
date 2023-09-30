add_library(vkgfx
    "pch.h"

    "Context.h"
    "Context.cpp"
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

demo_set_common_properties(vkgfx)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #
elseif(MSVC)
    target_compile_options(vkgfx PRIVATE /wd4324) # structure was padded due to alignment specifier
endif()

if(DEMO_PCH)
    target_precompile_headers(vkgfx PRIVATE "pch.h")
endif()

target_link_libraries(vkgfx
    vko
    common
)
