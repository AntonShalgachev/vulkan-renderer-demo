set(PROJECT_NAME renderer)

set(ALL_FILES
    "pch.h"

    "wrapper/Assert.h"
    "wrapper/Assert.cpp"
    "wrapper/Buffer.h"
    "wrapper/Buffer.cpp"
    "wrapper/CommandBuffers.h"
    "wrapper/CommandBuffers.cpp"
    "wrapper/CommandPool.h"
    "wrapper/CommandPool.cpp"
    "wrapper/DebugMessage.h"
    "wrapper/DescriptorPool.h"
    "wrapper/DescriptorPool.cpp"
    "wrapper/DescriptorSetLayout.h"
    "wrapper/DescriptorSetLayout.cpp"
    "wrapper/DescriptorSets.h"
    "wrapper/DescriptorSets.cpp"
    "wrapper/Device.h"
    "wrapper/Device.cpp"
    "wrapper/DeviceMemory.h"
    "wrapper/DeviceMemory.cpp"
    "wrapper/Fence.h"
    "wrapper/Fence.cpp"
    "wrapper/Framebuffer.h"
    "wrapper/Framebuffer.cpp"
    "wrapper/Image.h"
    "wrapper/Image.cpp"
    "wrapper/ImageView.h"
    "wrapper/ImageView.cpp"
    "wrapper/Instance.h"
    "wrapper/Instance.cpp"
    "wrapper/PhysicalDevice.h"
    "wrapper/PhysicalDevice.cpp"
    "wrapper/Pipeline.h"
    "wrapper/Pipeline.cpp"
    "wrapper/PipelineLayout.h"
    "wrapper/PipelineLayout.cpp"
    "wrapper/Queue.h"
    "wrapper/Queue.cpp"
    "wrapper/QueueFamily.h"
    "wrapper/QueueFamily.cpp"
    "wrapper/RenderPass.h"
    "wrapper/RenderPass.cpp"
    "wrapper/Sampler.h"
    "wrapper/Sampler.cpp"
    "wrapper/Semaphore.h"
    "wrapper/Semaphore.cpp"
    "wrapper/ShaderModule.h"
    "wrapper/ShaderModule.cpp"
    "wrapper/Surface.h"
    "wrapper/Surface.cpp"
    "wrapper/Swapchain.h"
    "wrapper/Swapchain.cpp"
    "wrapper/UniqueHandle.h"
    "wrapper/UniqueHandle.cpp"
    "wrapper/Window.h"
    "wrapper/Window.cpp"
    
    "vkgfx/Buffer.h"
    "vkgfx/Buffer.cpp"
    "vkgfx/BufferMetadata.h"
    "vkgfx/Image.h"
    "vkgfx/Image.cpp"
    "vkgfx/ImageMetadata.h"
    "vkgfx/Texture.h"
    "vkgfx/Material.h"
    "vkgfx/Mesh.h"
    "vkgfx/PipelineKey.h"
    "vkgfx/PipelineKey.cpp"
    "vkgfx/Handles.h"
    "vkgfx/ResourceManager.h"
    "vkgfx/ResourceManager.cpp"
    "vkgfx/Renderer.h"
    "vkgfx/Renderer.cpp"

    "vkgfx/ResourceContainer.h"
    "vkgfx/ResourceContainer.cpp"
    "vkgfx/TestObject.h"

    "Application.cpp"
    "Application.h"
    "PhysicalDeviceSurfaceContainer.cpp"
    "PhysicalDeviceSurfaceContainer.h"
    "PhysicalDeviceSurfaceParameters.cpp"
    "PhysicalDeviceSurfaceParameters.h"
    "QueueFamilyIndices.cpp"
    "QueueFamilyIndices.h"
)

add_library(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${CMAKE_SOURCE_DIR}/include"

    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PUBLIC -Wno-defaulted-function-deleted)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.h")

target_link_libraries(${PROJECT_NAME}
    Vulkan::Vulkan
    common
    VulkanMemoryAllocator
)
