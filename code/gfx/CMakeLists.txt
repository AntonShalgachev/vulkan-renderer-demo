add_library(gfx
    "include/gfx/backend.h"
    "include/gfx/renderer.h"
    "include/gfx/resources.h"
    
    "src/backend.cpp"
    "src/renderer.cpp"
)

target_compile_features(gfx PUBLIC cxx_std_20)

target_include_directories(gfx PUBLIC
    "include"
)

target_link_libraries(gfx
    nstl
)
