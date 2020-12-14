#include "VulkanSurfaceCreator.h"

#include <GLFW/glfw3.h>
#include "Instance.h"
#include "Window.h"

VkSurfaceKHR vkr::VulkanSurfaceCreator::createVulkanSurface(Instance const& instance, Window const& window)
{
    VkSurfaceKHR handle = VK_NULL_HANDLE;

    if (glfwCreateWindowSurface(instance.getHandle(), window.m_handle, nullptr, &handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");

    return handle;
}
