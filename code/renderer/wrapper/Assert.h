#pragma once

#include <vulkan/vulkan.h>
#include <cassert>

#ifdef NDEBUG
#define VKO_ASSERT(cmd) (cmd)
#else
#define VKO_ASSERT(cmd) (assert((cmd) == VK_SUCCESS))
#endif
