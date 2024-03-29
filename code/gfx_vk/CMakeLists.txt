find_package(Vulkan REQUIRED)

add_library(gfx_vk
    "include/gfx_vk/backend.h"
    "include/gfx_vk/config.h"
    "include/gfx_vk/surface_factory.h"

    "src/pch.h"
    "src/backend.cpp"
    "src/context.h"
    "src/context.cpp"
    "src/conversions.h"
    "src/conversions.cpp"
    "src/resource_container.h"
    "src/resource_container.cpp"
    "src/descriptor_allocator.h"
    "src/descriptor_allocator.cpp"
    "src/renderer.h"
    "src/renderer.cpp"
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
    "src/utils.h"
    "src/utils.cpp"
    "src/instance.h"
    "src/instance.cpp"
    "src/memory.h"
    "src/memory.cpp"
    "src/transfers.cpp"
    "src/transfers.h"
    "src/command_pool.h"
    "src/command_pool.cpp"
)

demo_set_common_properties(gfx_vk)

target_include_directories(gfx_vk PUBLIC
    "include"
)

target_link_libraries(gfx_vk
    gfx
    Vulkan::Vulkan
    logging
)

target_precompile_headers(gfx_vk PRIVATE "src/pch.h")
