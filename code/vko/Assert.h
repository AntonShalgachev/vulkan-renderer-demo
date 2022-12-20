#pragma once

#include <vulkan/vulkan.h>

#ifdef NDEBUG
#define VKO_ASSERT(cmd) (cmd)
#else
#include <assert.h>
#define VKO_ASSERT(cmd) (assert((cmd) == VK_SUCCESS))
#endif
