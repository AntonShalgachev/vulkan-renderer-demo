#pragma once

#include "command_pool.h"

#include "nstl/blob_view.h"
#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
    struct allocation_handle;

    class context;

    struct transfer_data
    {
        size_t index = 0;

        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize buffer_offset = 0;

        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    };

    class transfers
    {
    public:
        transfers(context& context);
        ~transfers();

        transfer_data begin_transfer(nstl::blob_view bytes);
        void submit_and_wait(size_t index);

    private:
        context& m_context;

        command_pool m_command_pool;
        VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;

        nstl::vector<allocation_handle> m_allocations;

        struct transfer_storage;
        nstl::vector<transfer_storage> m_transfers;
    };
}
