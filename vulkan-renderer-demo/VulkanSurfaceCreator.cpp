#include "VulkanSurfaceCreator.h"

#include <GLFW/glfw3.h>
#include "Instance.h"
#include "Window.h"

vkr::UniqueHandle<VkSurfaceKHR> vkr::VulkanSurfaceCreator::createVulkanSurface(Instance const& instance, Window const& window)
{
    UniqueHandle<VkSurfaceKHR> handle;

    if (glfwCreateWindowSurface(instance.getHandle(), window.m_handle, nullptr, &handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");

    return handle;
}
