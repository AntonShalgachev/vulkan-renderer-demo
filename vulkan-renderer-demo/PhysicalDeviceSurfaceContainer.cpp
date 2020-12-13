#include "PhysicalDeviceSurfaceContainer.h"

vkr::PhysicalDeviceSurfaceContainer::PhysicalDeviceSurfaceContainer(PhysicalDevice&& physicalDdevice, Surface const& surface)
    : m_physicalDevice(std::move(physicalDdevice))
    , m_surface(surface)
    , m_parameters(m_physicalDevice, m_surface)
{
    
}

vkr::PhysicalDeviceSurfaceContainer::PhysicalDeviceSurfaceContainer(PhysicalDeviceSurfaceContainer&& rhs)
    : m_physicalDevice(std::move(rhs.m_physicalDevice))
    , m_surface(rhs.m_surface)
    , m_parameters(std::move(rhs.m_parameters))
{

}
