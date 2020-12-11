#include "PhysicalDevice.h"

vkr::PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle, Surface const& surface) : m_handle(handle)
{

}

VkPhysicalDeviceProperties const& vkr::PhysicalDevice::getProperties() const
{
    if (!m_properties.has_value())
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_handle, &properties);
        m_properties = properties;
    }

    return m_properties.value();
}

VkPhysicalDeviceFeatures const& vkr::PhysicalDevice::getFeatures() const
{
    if (!m_features.has_value())
    {
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(m_handle, &features);
        m_features = features;
    }

    return m_features.value();
}
