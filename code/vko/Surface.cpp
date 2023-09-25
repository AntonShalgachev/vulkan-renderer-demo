#include "Surface.h"

#include "vko/Instance.h"
#include "vko/Window.h"

vko::Surface::Surface(VkSurfaceKHR handle, Instance const& instance)
    : m_handle(handle)
    , m_instance(instance)
{
    
}

vko::Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance.getHandle(), m_handle, nullptr);
}
