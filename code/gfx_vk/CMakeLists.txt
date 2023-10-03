add_library(gfx_vk
    "include/gfx_vk/backend.h"
    "include/gfx_vk/config.h"
    
    "src/backend.cpp"
    "src/context.h"
    "src/context.cpp"
    "src/conversions.h"
    "src/conversions.cpp"
    "src/resource_container.h"
    "src/resource_container.cpp"
    "src/descriptor_allocator.h"
    "src/descriptor_allocator.cpp"

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
    "src/descriptor_set_layout.h"
    "src/descriptor_set_layout.cpp"
    "src/pipeline_layout.h"
    "src/pipeline_layout.cpp"
    "src/renderstate.h"
    "src/renderstate.cpp"
    "src/renderpass.h"
    "src/renderpass.cpp"
    "src/framebuffer.h"
    "src/framebuffer.cpp"
    "src/descriptorgroup.h"
    "src/descriptorgroup.cpp"
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
