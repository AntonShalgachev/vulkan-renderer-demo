find_package(Vulkan REQUIRED)

add_library(vko
    "pch.h"
    
    "Allocator.h"
    "Allocator.cpp"
    "Assert.h"
    "Buffer.h"
    "Buffer.cpp"
    "CommandBuffers.h"
    "CommandBuffers.cpp"
    "CommandPool.h"
    "CommandPool.cpp"
    "DebugMessage.h"
    "DescriptorPool.h"
    "DescriptorPool.cpp"
    "DescriptorSetLayout.h"
    "DescriptorSetLayout.cpp"
    "DescriptorSets.h"
    "DescriptorSets.cpp"
    "Device.h"
    "Device.cpp"
    "DeviceMemory.h"
    "DeviceMemory.cpp"
    "Fence.h"
    "Fence.cpp"
    "Framebuffer.h"
    "Framebuffer.cpp"
    "Image.h"
    "Image.cpp"
    "ImageView.h"
    "ImageView.cpp"
    "Instance.h"
    "Instance.cpp"
    "PhysicalDevice.h"
    "PhysicalDevice.cpp"
    "PhysicalDeviceSurfaceParameters.h"
    "PhysicalDeviceSurfaceParameters.cpp"
    "Pipeline.h"
    "Pipeline.cpp"
    "PipelineLayout.h"
    "PipelineLayout.cpp"
    "Queue.h"
    "Queue.cpp"
    "QueueFamily.h"
    "QueueFamily.cpp"
    "RenderPass.h"
    "RenderPass.cpp"
    "Sampler.h"
    "Sampler.cpp"
    "SamplerProperties.h"
    "Semaphore.h"
    "Semaphore.cpp"
    "ShaderModule.h"
    "ShaderModule.cpp"
    "ShaderModuleProperties.h"
    "Surface.h"
    "Surface.cpp"
    "Swapchain.h"
    "Swapchain.cpp"
    "UniqueHandle.h"
    "Window.h"
)

target_include_directories(vko PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

demo_set_common_properties(vko)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vko PUBLIC -Wno-cast-function-type-strict)
    target_compile_options(vko PUBLIC -Wno-defaulted-function-deleted)
endif()

if(DEMO_PCH)
    target_precompile_headers(vko PRIVATE "pch.h")
endif()

target_link_libraries(vko
    Vulkan::Vulkan
    nstl
    memory
)
