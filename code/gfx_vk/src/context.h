#pragma once

#include "resource_container.h"

#include "vko/Instance.h"
#include "vko/Surface.h"
#include "vko/PhysicalDevice.h"
#include "vko/Device.h"
#include "vko/Window.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"
#include "vko/CommandPool.h"

#include "nstl/unique_ptr.h"

namespace vko
{
    class Queue;
}

namespace gfx_vk
{
    class context
    {
    public:
        context(vko::Window const& window, char const* name, bool enable_validation);
        ~context();

        vko::Instance const& get_instance() const;
        vko::Surface const& get_surface() const;
        vko::Device const& get_device() const;

        vko::PhysicalDevice const& get_physical_device() const;
        vko::PhysicalDeviceSurfaceParameters const& get_physical_device_surface_parameters() const;

        vko::Queue const& get_transfer_queue() const;
        vko::CommandPool const& get_transfer_command_pool() const;

        resource_container& get_resources() { return m_resources; }
        resource_container const& get_resources() const { return m_resources; }

        void on_surface_changed();

    private:
        vko::Instance m_instance;
        vko::Surface m_surface;

        vko::PhysicalDevice m_physical_device;
        vko::PhysicalDeviceSurfaceParameters m_params;

        vko::Device m_device;

        vko::CommandPool m_transfer_command_pool;

        resource_container m_resources;
    };
}
