#include "PhysicalDeviceSurfaceParameters.h"

#include "wrapper/Assert.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Surface.h"
#include "QueueFamilyIndices.h"

vkr::PhysicalDeviceSurfaceParameters::PhysicalDeviceSurfaceParameters(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    : m_physicalDevice(physicalDevice)
    , m_surface(surface)
    , m_queueFamilyIndices(physicalDevice, *this)
{
    queryCapabilities();
    queryFormats();
    queryPresentModes();
    queryPresentationSupport();
}

vkr::PhysicalDeviceSurfaceParameters::~PhysicalDeviceSurfaceParameters() = default;

vkr::PhysicalDeviceSurfaceParameters::PhysicalDeviceSurfaceParameters(PhysicalDeviceSurfaceParameters&&) = default;

bool vkr::PhysicalDeviceSurfaceParameters::isPresentationSupported(vko::QueueFamily const& queueFamily) const
{
    uint32_t index = queueFamily.getIndex();

    if (index >= m_queuePresentationSupport.size())
        return false;

    return m_queuePresentationSupport[index];
}

void vkr::PhysicalDeviceSurfaceParameters::onSurfaceChanged()
{
    queryCapabilities();
}

void vkr::PhysicalDeviceSurfaceParameters::queryCapabilities()
{
    VKO_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &m_capabilities));
}

void vkr::PhysicalDeviceSurfaceParameters::queryFormats()
{
    uint32_t count = 0;
    VKO_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, nullptr));

    if (count > 0)
    {
        m_formats.resize(count);
        VKO_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, m_formats.data()));
    }
}

void vkr::PhysicalDeviceSurfaceParameters::queryPresentModes()
{
    uint32_t count = 0;
    VKO_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, nullptr));

    if (count > 0)
    {
        m_presentModes.resize(count);
        VKO_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice.getHandle(), m_surface.getHandle(), &count, m_presentModes.data()));
    }
}

void vkr::PhysicalDeviceSurfaceParameters::queryPresentationSupport()
{
    nstl::vector<vko::QueueFamily> const& queueFamilies = m_physicalDevice.getQueueFamilies();

    m_queuePresentationSupport.resize(queueFamilies.size());

    for (vko::QueueFamily const& queueFamily : queueFamilies)
    {
        uint32_t index = queueFamily.getIndex();

        VkBool32 presentSupport = false;
        VKO_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice.getHandle(), index, m_surface.getHandle(), &presentSupport));

        if (index >= m_queuePresentationSupport.size())
            m_queuePresentationSupport.resize(index + 1);

        m_queuePresentationSupport[index] = presentSupport > 0;
    }
}
