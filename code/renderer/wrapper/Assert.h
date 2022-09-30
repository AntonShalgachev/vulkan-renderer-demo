#pragma once

#include <vulkan/vulkan.h>
#include <cassert>

#define VKO_ASSERT(cmd) (assert((cmd) == VK_SUCCESS))
