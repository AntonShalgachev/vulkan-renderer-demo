#pragma once

#include <vulkan/vulkan.h>

#ifdef NDEBUG
#define VKO_VERIFY(cmd) (cmd)
#else
#include <assert.h>
namespace vko
{
    inline void do_assert(VkResult result, char const* command)
    {
        assert(result == VK_SUCCESS);
    }
}
#define VKO_VERIFY(cmd) vko::do_assert((cmd), #cmd)
#endif
