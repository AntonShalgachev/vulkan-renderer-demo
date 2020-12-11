#include "Surface.h"
#include "Instance.h"

vkr::Surface::Surface(Instance const& instance, GLFWwindow* window) : m_instance(instance)
{
    if (glfwCreateWindowSurface(m_instance.getHandle(), window, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}

vkr::Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance.getHandle(), m_handle, nullptr);
}
