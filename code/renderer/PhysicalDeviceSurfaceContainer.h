#pragma once

#include "renderer/wrapper/PhysicalDevice.h"
#include "renderer/wrapper/Surface.h"

#include "renderer/PhysicalDeviceSurfaceParameters.h"

namespace vkr
{
    class PhysicalDeviceSurfaceContainer
    {
    public:
    	PhysicalDeviceSurfaceContainer(vko::PhysicalDevice&& physicalDdevice, vko::Surface const& surface);

        PhysicalDeviceSurfaceContainer(PhysicalDeviceSurfaceContainer const&) = delete;
        PhysicalDeviceSurfaceContainer(PhysicalDeviceSurfaceContainer&&);
        PhysicalDeviceSurfaceContainer& operator=(PhysicalDeviceSurfaceContainer const&) = delete;
        PhysicalDeviceSurfaceContainer& operator=(PhysicalDeviceSurfaceContainer&&) = delete;

        vko::PhysicalDevice const& getPhysicalDevice() const { return m_physicalDevice; }
        PhysicalDeviceSurfaceParameters const& getParameters() const { return m_parameters; }
        PhysicalDeviceSurfaceParameters& getParameters() { return m_parameters; }

    private:
        vko::PhysicalDevice m_physicalDevice;
        vko::Surface const& m_surface;
        PhysicalDeviceSurfaceParameters m_parameters;
    };
}
