#include "memory.h"

#include "context.h"

namespace
{
    uint32_t find_memory_type(gfx_vk::context const& context, uint32_t type_bits, VkMemoryPropertyFlags flags)
    {
        VkPhysicalDeviceMemoryProperties const& props = context.get_physical_device_props().memory_properties;

        for (uint32_t i = 0; i < props.memoryTypeCount; i++)
        {
            if ((type_bits & (1 << i)) == 0)
                continue;

            if ((props.memoryTypes[i].propertyFlags & flags) != flags)
                continue;

            return i;
        }

        assert(false);
        return 0;
    }

    bool is_mappable(VkMemoryPropertyFlags flags)
    {
        return (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
}

struct gfx_vk::memory::allocation
{
    unique_handle<VkDeviceMemory> handle;
    allocation_data data;
};

gfx_vk::memory::memory(context& context) : m_context(context)
{

}

gfx_vk::memory::~memory() = default;

gfx_vk::allocation_handle gfx_vk::memory::allocate(VkMemoryRequirements requirements, VkMemoryPropertyFlags flags)
{
    allocation_handle handle = { m_allocations.size() };
    allocation& alloc = m_allocations.emplace_back();

    VkMemoryAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = find_memory_type(m_context, requirements.memoryTypeBits, flags),
    };
    GFX_VK_VERIFY(vkAllocateMemory(m_context.get_device_handle(), &info, &m_context.get_allocator(), &alloc.handle.get()));
    
    alloc.data = {
        .handle = alloc.handle,
        .offset = 0,
        .ptr = nullptr,
    };

    if (is_mappable(flags))
        GFX_VK_VERIFY(vkMapMemory(m_context.get_device_handle(), alloc.handle, 0, requirements.size, 0, &alloc.data.ptr));

    return handle;
}

gfx_vk::allocation_data const* gfx_vk::memory::get_data(allocation_handle handle) const
{
    if (!handle)
        return nullptr;

    return &get_allocation(handle).data;
}

void gfx_vk::memory::free(allocation_handle handle)
{
    if (!handle)
        return;

    allocation& alloc = get_allocation(handle);

    if (alloc.handle == VK_NULL_HANDLE)
        return;

    if (alloc.data.ptr)
        vkUnmapMemory(m_context.get_device_handle(), alloc.handle);

    vkFreeMemory(m_context.get_device_handle(), alloc.handle, &m_context.get_allocator());
    alloc.handle = VK_NULL_HANDLE;
}

gfx_vk::memory::allocation& gfx_vk::memory::get_allocation(allocation_handle handle)
{
    assert(handle.index);
    return m_allocations[*handle.index];
}

gfx_vk::memory::allocation const& gfx_vk::memory::get_allocation(allocation_handle handle) const
{
    assert(handle.index);
    return m_allocations[*handle.index];
}
