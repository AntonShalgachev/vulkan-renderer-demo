#include "gfx_vk_win64/surface_factory.h"

#include "platform_win64/window.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

gfx_vk_win64::surface_factory::surface_factory(platform::window_handle_t window)
    : m_window(window)
{

}

nstl::span<char const* const> gfx_vk_win64::surface_factory::get_instance_extensions()
{
    uint32_t count = 0;
    const char** extensions = nullptr;
    extensions = glfwGetRequiredInstanceExtensions(&count);

    return { extensions, count };
}

VkResult gfx_vk_win64::surface_factory::create(VkInstance instance, VkSurfaceKHR* handle)
{
    return glfwCreateWindowSurface(instance, platform_win64::get_native_window_handle(m_window), nullptr, handle);
}
