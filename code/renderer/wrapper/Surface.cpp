#include "Surface.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

#include "Instance.h"
#include "Window.h"

vko::Surface::Surface(VkSurfaceKHR handle, Instance const& instance, vko::Window const& window)
    : m_handle(handle)
    , m_instance(instance)
    , m_window(window)
{
    
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
