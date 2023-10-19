#include "platform_win64/window.h"

#include <GLFW/glfw3.h>

platform::window_handle_t platform_win64::create_window_handle(GLFWwindow* handle)
{
    return static_cast<platform::window_handle_t>(handle);
}

GLFWwindow* platform_win64::get_native_window_handle(platform::window_handle_t handle)
{
    return static_cast<GLFWwindow*>(handle);
}
