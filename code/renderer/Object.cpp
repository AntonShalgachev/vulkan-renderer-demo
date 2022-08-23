#include "Object.h"
#include "Application.h"

vko::Instance const& vkr::Object::getInstance() const
{
    return getApp().getInstance();
}

vko::Surface const& vkr::Object::getSurface() const
{
    return getApp().getSurface();
}

vko::Device const& vkr::Object::getDevice() const
{
    return getApp().getDevice();
}

vko::PhysicalDevice const& vkr::Object::getPhysicalDevice() const
{
    return getApp().getPhysicalDevice();
}

vkr::PhysicalDeviceSurfaceParameters const& vkr::Object::getPhysicalDeviceSurfaceParameters() const
{
    return getApp().getPhysicalDeviceSurfaceParameters();
}
