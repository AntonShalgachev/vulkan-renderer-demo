#include "PhysicalDeviceSurfaceContainer.h"

vkr::PhysicalDeviceSurfaceContainer::PhysicalDeviceSurfaceContainer(vko::PhysicalDevice&& physicalDdevice, vko::Surface const& surface)
    : m_physicalDevice(std::move(physicalDdevice))
    , m_surface(surface)
    , m_parameters(m_physicalDevice, m_surface)
{
    
}

vkr::PhysicalDeviceSurfaceContainer::PhysicalDeviceSurfaceContainer(PhysicalDeviceSurfaceContainer&& rhs) = default;
