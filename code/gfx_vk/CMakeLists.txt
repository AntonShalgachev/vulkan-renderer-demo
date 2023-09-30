add_library(gfx_vk
    "include/gfx_vk/backend.h"
    
    "src/backend.cpp"
    "src/context.h"
    "src/context.cpp"
    "src/swapchain.h"
    "src/swapchain.cpp"
    "src/buffer.h"
    "src/buffer.cpp"
    "src/image.h"
    "src/image.cpp"
    "src/sampler.h"
    "src/sampler.cpp"
    "src/shader.h"
    "src/shader.cpp"
)

demo_set_common_properties(gfx_vk)

target_include_directories(gfx_vk PUBLIC
    "include"
)

target_link_libraries(gfx_vk
    gfx
    vko # TODO remove
    logging
)
