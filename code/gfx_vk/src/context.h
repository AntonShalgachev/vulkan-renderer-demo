#pragma once

#include "nstl/unique_ptr.h"

namespace vko
{
    class Instance;
    class Surface;
    class PhysicalDevice;
    class Device;
    class Window;
    struct PhysicalDeviceSurfaceParameters;
    class CommandPool;
    class Queue;
}

namespace gfx_vk
{
    struct context_impl;

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

        void on_surface_changed();

    private:
        nstl::unique_ptr<context_impl> m_impl;
    };
}
