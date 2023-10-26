#pragma once

#include "nstl/optional.h"
#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    class context;

    struct allocation_handle
    {
        nstl::optional<size_t> index = 0;

        explicit operator bool() const { return index.has_value(); }
    };

    struct allocation_data
    {
        VkDeviceMemory handle = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
        void* ptr = nullptr;
    };

    class memory
    {
    public:
        memory(context& context);
        ~memory();

        allocation_handle allocate(VkMemoryRequirements requirements, VkMemoryPropertyFlags flags);
        allocation_data const* get_data(allocation_handle handle) const;
        void free(allocation_handle handle);

    private:
        struct allocation;

    private:
        allocation& get_allocation(allocation_handle handle);
        allocation const& get_allocation(allocation_handle handle) const;

    private:
        context& m_context;

        nstl::vector<allocation> m_allocations;
    };
}
