#include "PhysicalDeviceSurfaceParameters.h"
#include "PhysicalDevice.h"
#include "Surface.h"

vkr::PhysicalDeviceSurfaceParameters::PhysicalDeviceSurfaceParameters(PhysicalDevice const& physicalDevice, Surface const& surface)
{
    VkPhysicalDevice physicalDeviceHandle = physicalDevice.getHandle();
    VkSurfaceKHR surfaceHandle = surface.getHandle();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceHandle, surfaceHandle, &m_capabilities);

    {
        uint32_t count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, surfaceHandle, &count, nullptr);

        if (count > 0)
        {
            m_formats.resize(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, surfaceHandle, &count, m_formats.data());
        }
    }

    {
        uint32_t count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, surfaceHandle, &count, nullptr);

        if (count > 0)
        {
            m_presentModes.resize(count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, surfaceHandle, &count, m_presentModes.data());
        }
    }

    std::vector<VkQueueFamilyProperties> const& queueFamilies = physicalDevice.getQueueFamilyProperties();
    m_queuePresentationSupport.resize(queueFamilies.size());

    for (auto i = 0; i < queueFamilies.size(); i++)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.getHandle(), i, surfaceHandle, &presentSupport);

        m_queuePresentationSupport[i] = presentSupport;
    }
}
