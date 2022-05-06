#include "Surface.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

#include "Instance.h"
#include "Window.h"

vkr::Surface::Surface(Instance const& instance, Window const& window) : m_instance(instance), m_window(window)
{
    if (glfwCreateWindowSurface(instance.getHandle(), window.getHandle(), nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}

vkr::Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance.getHandle(), m_handle, nullptr);
}

int vkr::Surface::getWidth() const
{
    return m_window.getWidth();
}

int vkr::Surface::getHeight() const
{
    return m_window.getHeight();
}
