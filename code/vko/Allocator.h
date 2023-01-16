#pragma once

#include "nstl/string_view.h"

#include <vulkan/vulkan.h>

namespace vko
{
    enum class AllocatorScope
    {
        Buffer,
        CommandPool,
        DescriptorPool,
        DescriptorSetLayout,
        Device,
        DeviceMemory,
        Fence,
        Framebuffer,
        Image,
        ImageView,
        Instance,
        Debug,
        Pipeline,
        PipelineLayout,
        RenderPass,
        Sampler,
        Semaphore,
        ShaderModule,
        Swapchain,
    };

    class Allocator
    {
    public:
        Allocator(AllocatorScope scope);

        operator VkAllocationCallbacks const* () const;

    private:
        static void* allocate(void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void* reallocate(void* userData, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
        static void deallocate(void* userData, void* ptr);

    private:
        VkAllocationCallbacks m_callbacks{};
    };
}
