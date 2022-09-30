#pragma once

#include <vulkan/vulkan.h>

// TODO move somewhere
#include <string>
#include <stdexcept>
#define VKR_ASSERT(cmd) do { if (auto res = (cmd); res != VK_SUCCESS) throw std::runtime_error(std::to_string(res)); } while(0)
