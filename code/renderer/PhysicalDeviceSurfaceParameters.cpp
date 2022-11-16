#include "PhysicalDeviceSurfaceParameters.h"

#include "wrapper/Assert.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Surface.h"
#include "QueueFamilyIndices.h"

namespace
{
    VkSurfaceCapabilitiesKHR queryCapabilities(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        VkSurfaceCapabilitiesKHR result{};
        VKO_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.getHandle(), surface.getHandle(), &result));
        return result;
    }

    nstl::vector<VkSurfaceFormatKHR> queryFormats(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        uint32_t count = 0;
        VKO_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getHandle(), &count, nullptr));

        nstl::vector<VkSurfaceFormatKHR> result;

        if (count > 0)
        {
            result.resize(count);
            VKO_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getHandle(), &count, result.data()));
        }

        return result;
    }

    nstl::vector<VkPresentModeKHR> queryPresentModes(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        uint32_t count = 0;
        VKO_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getHandle(), &count, nullptr));

        nstl::vector<VkPresentModeKHR> result;

        if (count > 0)
        {
            result.resize(count);
            VKO_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getHandle(), &count, result.data()));
        }

        return result;
    }

    nstl::vector<bool> queryPresentationSupport(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        nstl::vector<vko::QueueFamily> const& queueFamilies = physicalDevice.getQueueFamilies();

        nstl::vector<bool> result;

        result.resize(queueFamilies.size());

        for (vko::QueueFamily const& queueFamily : queueFamilies)
        {
            uint32_t index = queueFamily.getIndex();

            VkBool32 presentSupport = false;
            VKO_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.getHandle(), index, surface.getHandle(), &presentSupport));

            if (index >= result.size())
                result.resize(index + 1);

            result[index] = presentSupport > 0;
        }

        return result;
    }
}

vkr::PhysicalDeviceSurfaceParameters::PhysicalDeviceSurfaceParameters(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    : m_physicalDevice(physicalDevice)
    , m_surface(surface)
    , m_capabilities(queryCapabilities(physicalDevice, surface))
    , m_formats(queryFormats(physicalDevice, surface))
    , m_presentModes(queryPresentModes(physicalDevice, surface))
    , m_queuePresentationSupport(queryPresentationSupport(physicalDevice, surface))
    , m_queueFamilyIndices(physicalDevice, *this)
{

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
    m_capabilities = queryCapabilities(m_physicalDevice, m_surface);
}
