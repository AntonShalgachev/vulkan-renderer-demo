add_library(vko
    "pch.h"

    "Assert.h"
    "Assert.cpp"
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
    "UniqueHandle.cpp"
    "Window.h"
    "Window.cpp"
)

target_include_directories(vko PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_features(vko PUBLIC cxx_std_20)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vko PUBLIC -Wno-defaulted-function-deleted)
endif()

target_precompile_headers(vko PRIVATE "pch.h")

target_link_libraries(vko
    Vulkan::Vulkan
    nstl
)
