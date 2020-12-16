#include "Object.h"
#include "Application.h"

vkr::Instance const& vkr::Object::getInstance() const
{
    return getApp().getInstance();
}

vkr::Surface const& vkr::Object::getSurface() const
{
    return getApp().getSurface();
}

vkr::Device const& vkr::Object::getDevice() const
{
    return getApp().getDevice();
}
