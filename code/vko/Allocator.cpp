#include "Allocator.h"

#include "nstl/unordered_map.h"

#include "memory/tracking.h"

#include "logging/logging.h"

namespace
{
    nstl::string_view getScopeName(vko::AllocatorScope scope)
    {
#define VKO_MEMORY_SCOPE_NAME(name) ("Rendering/Vulkan/" name)

        switch (scope)
        {
        case vko::AllocatorScope::Buffer:
            return VKO_MEMORY_SCOPE_NAME("Buffer");
        case vko::AllocatorScope::CommandPool:
            return VKO_MEMORY_SCOPE_NAME("CommandPool");
        case vko::AllocatorScope::DescriptorPool:
            return VKO_MEMORY_SCOPE_NAME("DescriptorPool");
        case vko::AllocatorScope::DescriptorSetLayout:
            return VKO_MEMORY_SCOPE_NAME("DescriptorSetLayout");
        case vko::AllocatorScope::Device:
            return VKO_MEMORY_SCOPE_NAME("Device");
        case vko::AllocatorScope::DeviceMemory:
            return VKO_MEMORY_SCOPE_NAME("DeviceMemory");
        case vko::AllocatorScope::Fence:
            return VKO_MEMORY_SCOPE_NAME("Fence");
        case vko::AllocatorScope::Framebuffer:
            return VKO_MEMORY_SCOPE_NAME("Framebuffer");
        case vko::AllocatorScope::Image:
            return VKO_MEMORY_SCOPE_NAME("Image");
        case vko::AllocatorScope::ImageView:
            return VKO_MEMORY_SCOPE_NAME("ImageView");
        case vko::AllocatorScope::Instance:
            return VKO_MEMORY_SCOPE_NAME("Instance");
        case vko::AllocatorScope::Debug:
            return VKO_MEMORY_SCOPE_NAME("Debug");
        case vko::AllocatorScope::Pipeline:
            return VKO_MEMORY_SCOPE_NAME("Pipeline");
        case vko::AllocatorScope::PipelineLayout:
            return VKO_MEMORY_SCOPE_NAME("PipelineLayout");
        case vko::AllocatorScope::RenderPass:
            return VKO_MEMORY_SCOPE_NAME("RenderPass");
        case vko::AllocatorScope::Sampler:
            return VKO_MEMORY_SCOPE_NAME("Sampler");
        case vko::AllocatorScope::Semaphore:
            return VKO_MEMORY_SCOPE_NAME("Semaphore");
        case vko::AllocatorScope::ShaderModule:
            return VKO_MEMORY_SCOPE_NAME("ShaderModule");
        case vko::AllocatorScope::Swapchain:
            return VKO_MEMORY_SCOPE_NAME("Swapchain");
        }

#undef VKO_MEMORY_SCOPE_NAME

        assert(false);
        return {};
    }

    memory::tracking::scope_id getScopeId(vko::AllocatorScope scope)
    {
        static nstl::unordered_map<vko::AllocatorScope, memory::tracking::scope_id> map;

        auto it = map.find(scope);
        if (it == map.end())
        {
            memory::tracking::scope_id id = memory::tracking::create_scope_id(getScopeName(scope), memory::tracking::scope_type::external);
            it = map.insert_or_assign(scope, id);
        }

        assert(it != map.end());

        return it->value();
    }
}

vko::Allocator::Allocator(AllocatorScope scope)
{
    memory::tracking::scope_id scopeId = getScopeId(scope);

    // TODO find a less hacky way to pass scopeId to the callback. Can't save `this` because the object can be "moved"
    static_assert(sizeof(m_callbacks.pUserData) >= sizeof(scopeId));
    void* userData = nullptr;
    memcpy(&userData, &scopeId, sizeof(scopeId));

    m_callbacks = {
        .pUserData = userData,
        .pfnAllocation = &allocate,
        .pfnReallocation = &reallocate,
        .pfnFree = &deallocate,
    };
}

vko::Allocator::operator VkAllocationCallbacks const* () const
{
    return &m_callbacks;
}

void* vko::Allocator::allocate(void* userData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
    // TODO don't allocate more memory just for size

    assert(userData);
    memory::tracking::scope_id scopeId;
    static_assert(sizeof(userData) >= sizeof(scopeId));
    memcpy(&scopeId, &userData, sizeof(scopeId));

    MEMORY_TRACKING_SCOPE(scopeId);

    // TODO make use of alignment
    assert(alignment <= alignof(max_align_t));

    void* originalPtr = ::operator new(size + sizeof(size_t));
    *static_cast<size_t*>(originalPtr) = size;

    void* ptr = static_cast<unsigned char*>(originalPtr) + sizeof(size_t);

    return ptr;
}

void* vko::Allocator::reallocate(void* userData, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
    // TODO implement properly

    assert(ptr);

    void* originalPtr = static_cast<unsigned char*>(ptr) - sizeof(size_t);
    size_t oldSize = *static_cast<size_t*>(originalPtr);

    if (size <= oldSize)
        return ptr;

    void* newPtr = allocate(userData, size, alignment, allocationScope);

    ::memcpy(newPtr, ptr, oldSize);

    deallocate(userData, ptr);

    return newPtr;
}

void vko::Allocator::deallocate(void* userData, void* ptr)
{
    if (!ptr)
        return;

    void* originalPtr = static_cast<unsigned char*>(ptr) - sizeof(size_t);

    return operator delete(originalPtr);
}
