add_library(gfx_vk
    "include/gfx_vk/backend.h"
    
    "src/backend.cpp"
)

target_compile_features(gfx_vk PUBLIC cxx_std_20)

target_include_directories(gfx_vk PUBLIC
    "include"
)

target_link_libraries(gfx_vk
    nstl
)
