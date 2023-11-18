#include "gfx_vk_win64/surface_factory.h"

#include "platform_win64/window.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <vulkan/vulkan_win32.h>

namespace
{
    constexpr char const* const REQUIRED_EXTENSIONS[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
    };
}

gfx_vk_win64::surface_factory::surface_factory(platform::window& window)
    : m_window(window)
{

}

nstl::span<char const* const> gfx_vk_win64::surface_factory::get_instance_extensions()
{
    return REQUIRED_EXTENSIONS;
}

VkResult gfx_vk_win64::surface_factory::create(VkInstance instance, VkAllocationCallbacks const* allocator, VkSurfaceKHR* handle)
{
    HWND window_handle = static_cast<platform_win64::win32_window&>(m_window).get_handle().get_as<HWND>();

    VkWin32SurfaceCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(nullptr),
        .hwnd = window_handle,
    };

    return vkCreateWin32SurfaceKHR(instance, &info, allocator, handle);
}
