#include "PhysicalDeviceSurfaceParameters.h"
#include "PhysicalDevice.h"
#include "Surface.h"

vkr::PhysicalDeviceSurfaceParameters::PhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, Surface const& surface) : m_physicalDevice(physicalDevice), m_surface(surface)
{
    queryCapabilities();
    queryFormats();
    queryPresentModes();
    queryPresentationSupport();
}

bool vkr::PhysicalDeviceSurfaceParameters::isPresentationSupported(std::size_t queueIndex) const
{
    if (queueIndex >= m_queuePresentationSupport.size())
        return false;

    return m_queuePresentationSupport[queueIndex];
}

void vkr::PhysicalDeviceSurfaceParameters::onSurfaceChanged()
{
    queryCapabilities();
}

void vkr::PhysicalDeviceSurfaceParameters::queryCapabilities()
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &m_capabilities);
}

void vkr::PhysicalDeviceSurfaceParameters::queryFormats()
{
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, nullptr);

    if (count > 0)
    {
        m_formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, m_formats.data());
    }
}

void vkr::PhysicalDeviceSurfaceParameters::queryPresentModes()
{
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, nullptr);

    if (count > 0)
    {
        m_presentModes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, m_presentModes.data());
    }
}

void vkr::PhysicalDeviceSurfaceParameters::queryPresentationSupport()
{
    std::vector<VkQueueFamilyProperties> const& queueFamilies = m_physicalDevice.getQueueFamilyProperties();
    m_queuePresentationSupport.resize(queueFamilies.size());

    for (uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice.getHandle(), i, m_surface.getHandle(), &presentSupport);

        m_queuePresentationSupport[i] = presentSupport;
    }
}
