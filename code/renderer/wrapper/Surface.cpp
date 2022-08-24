#include "Surface.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

#include "Instance.h"
#include "GlfwWindow.h"

vko::Surface::Surface(Instance const& instance, vkr::GlfwWindow const& window) : m_instance(instance), m_window(window)
{
    if (glfwCreateWindowSurface(instance.getHandle(), window.getHandle(), nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create window surface!");
}

vko::Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance.getHandle(), m_handle, nullptr);
}

int vko::Surface::getWidth() const
{
    return m_window.getWidth();
}

int vko::Surface::getHeight() const
{
    return m_window.getHeight();
}
