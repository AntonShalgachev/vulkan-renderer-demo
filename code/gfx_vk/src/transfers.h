#pragma once

#include "command_pool.h"
#include "memory.h"

#include "gfx/resources.h"

#include "nstl/blob_view.h"
#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace gfx_vk
{
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

        transfer_data begin_transfer(gfx::data_reader& reader);
        void submit_and_wait(size_t index);

    private:
        context& m_context;

        unique_handle<VkBuffer> m_buffer;
        allocation_handle m_memory;

        command_pool m_command_pool;
        VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;

        struct transfer_storage;
        nstl::vector<transfer_storage> m_transfers;
    };
}
