set(PROJECT_NAME librenderer)

set(ALL_FILES
    "glm.h"

    "Application.h"
    "Application.cpp"
    "Buffer.h"
    "Buffer.cpp"
    "Camera.h"
    "Camera.cpp"
    "CommandBuffer.h"
    "CommandBuffer.cpp"
    "CommandBuffers.h"
    "CommandBuffers.cpp"
    "CommandPool.h"
    "CommandPool.cpp"
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
    "Hash.h"
    "Hash.cpp"
    "Image.h"
    "Image.cpp"
    "ImageView.h"
    "ImageView.cpp"
    "Instance.h"
    "Instance.cpp"
    "Light.h"
    "Light.cpp"
    "Material.h"
    "Material.cpp"
    "Mesh.h"
    "Mesh.cpp"
    "Object.h"
    "Object.cpp"
    "ObjectInstance.h"
    "ObjectInstance.cpp"
    "PhysicalDevice.h"
    "PhysicalDevice.cpp"
    "PhysicalDeviceSurfaceContainer.h"
    "PhysicalDeviceSurfaceContainer.cpp"
    "PhysicalDeviceSurfaceParameters.h"
    "PhysicalDeviceSurfaceParameters.cpp"
    "Pipeline.h"
    "Pipeline.cpp"
    "PipelineConfiguration.h"
    "PipelineConfiguration.cpp"
    "PipelineLayout.h"
    "PipelineLayout.cpp"
    "Queue.h"
    "Queue.cpp"
    "QueueFamily.h"
    "QueueFamily.cpp"
    "QueueFamilyIndices.h"
    "QueueFamilyIndices.cpp"
    "Renderer.h"
    "Renderer.cpp"
    "RenderPass.h"
    "RenderPass.cpp"
    "Sampler.h"
    "Sampler.cpp"
    "SceneObject.h"
    "SceneObject.cpp"
    "ScopedOneTimeCommandBuffer.h"
    "ScopedOneTimeCommandBuffer.cpp"
    "Semaphore.h"
    "Semaphore.cpp"
    "Shader.h"
    "Shader.cpp"
    "ShaderModule.h"
    "ShaderModule.cpp"
    "Surface.h"
    "Surface.cpp"
    "Swapchain.h"
    "Swapchain.cpp"
    "Texture.h"
    "Texture.cpp"
    "Timer.h"
    "Timer.cpp"
    "Transform.h"
    "Transform.cpp"
    "UniqueHandle.h"
    "UniqueHandle.cpp"
    "Utils.h"
    "Utils.cpp"
    "VertexLayout.h"
    "VertexLayout.cpp"
    "VertexLayoutDescriptions.h"
    "VertexLayoutDescriptions.cpp"
    "VulkanSurfaceCreator.h"
    "VulkanSurfaceCreator.cpp"
    "Window.h"
    "Window.cpp"
)

add_library(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${CMAKE_SOURCE_DIR}/include"

    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

find_package(Vulkan)

add_dependencies(${PROJECT_NAME}
    Vulkan::Vulkan
    Vulkan::Headers
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_link_libraries(${PROJECT_NAME}
    libimgui

    Vulkan::Vulkan
)
