#pragma once

#include <vulkan/vulkan.h>

#ifdef NDEBUG
#define VKO_VERIFY(cmd) (cmd)
#else
#include <assert.h>
#define VKO_VERIFY(cmd) (assert((cmd) == VK_SUCCESS))
#endif
