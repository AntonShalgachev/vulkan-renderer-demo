#pragma once

#include "platform/window.h"

struct GLFWwindow;

namespace platform_win64
{
    platform::window_handle_t create_window_handle(GLFWwindow* handle);
    GLFWwindow* get_native_window_handle(platform::window_handle_t handle);
}
