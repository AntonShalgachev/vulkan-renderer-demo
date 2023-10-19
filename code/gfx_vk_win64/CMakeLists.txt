add_library(gfx_vk_win64
    "include/gfx_vk_win64/surface_factory.h"

    "src/surface_factory.cpp"
)

demo_set_common_properties(gfx_vk_win64)

target_include_directories(gfx_vk_win64 PUBLIC
    "include"
)

target_link_libraries(gfx_vk_win64
    gfx_vk
    platform_win64
)
