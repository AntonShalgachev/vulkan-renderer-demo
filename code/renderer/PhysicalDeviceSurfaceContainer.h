#pragma once

#include "wrapper/PhysicalDevice.h"
#include "wrapper/Surface.h"
#include "PhysicalDeviceSurfaceParameters.h"

namespace vkr
{
    class PhysicalDeviceSurfaceContainer
    {
    public:
    	PhysicalDeviceSurfaceContainer(PhysicalDevice&& physicalDdevice, Surface const& surface);

        PhysicalDeviceSurfaceContainer(PhysicalDeviceSurfaceContainer const&) = delete;
        PhysicalDeviceSurfaceContainer(PhysicalDeviceSurfaceContainer&&);
        PhysicalDeviceSurfaceContainer& operator=(PhysicalDeviceSurfaceContainer const&) = delete;
        PhysicalDeviceSurfaceContainer& operator=(PhysicalDeviceSurfaceContainer&&) = delete;

        PhysicalDevice const& getPhysicalDevice() const { return m_physicalDevice; }
        PhysicalDeviceSurfaceParameters const& getParameters() const { return m_parameters; }
        PhysicalDeviceSurfaceParameters& getParameters() { return m_parameters; }

    private:
        PhysicalDevice m_physicalDevice;
        Surface const& m_surface;
        PhysicalDeviceSurfaceParameters m_parameters;
    };
}
