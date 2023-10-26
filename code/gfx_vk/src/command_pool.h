#pragma once

#include "utils.h"

namespace gfx_vk
{
    class context;

    class command_pool
    {
    public:
        command_pool(context& context, uint32_t queue_family);
        command_pool(command_pool&&) = default;
        ~command_pool();

        command_pool& operator=(command_pool&& rhs) = default;

        VkCommandPool const& get_handle() const { return m_handle; }

        VkCommandBuffer allocate();

    private:
        context& m_context;
        unique_handle<VkCommandPool> m_handle;
    };
}
