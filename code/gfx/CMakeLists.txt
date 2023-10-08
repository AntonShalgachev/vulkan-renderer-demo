add_library(gfx
    "include/gfx/backend.h"
    "include/gfx/renderer.h"
    "include/gfx/resources.h"
    
    "src/backend.cpp"
    "src/renderer.cpp"
)

demo_set_common_properties(gfx)

target_include_directories(gfx PUBLIC
    "include"
)

target_link_libraries(gfx
    nstl
    tglm
)
