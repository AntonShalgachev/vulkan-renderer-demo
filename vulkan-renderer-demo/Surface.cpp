#include "Surface.h"
#include "Instance.h"
#include "VulkanSurfaceCreator.h"
#include "Window.h"

vkr::Surface::Surface(Instance const& instance, Window const& window) : m_instance(instance), m_window(window)
{
    m_handle = VulkanSurfaceCreator::createVulkanSurface(instance, window);
}

vkr::Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance.getHandle(), m_handle, nullptr);
}

vkr::Surface::Surface(Surface&&) = default;

int vkr::Surface::getWidth() const
{
    return m_window.getWidth();
}

int vkr::Surface::getHeight() const
{
    return m_window.getHeight();
}
