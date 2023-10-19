#pragma once

#include "gfx_vk/config.h"
#include "resource_container.h"
#include "descriptor_allocator.h"
#include "renderer.h"

#include "vko/Instance.h"
#include "vko/Surface.h"
#include "vko/PhysicalDevice.h"
#include "vko/Device.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"
#include "vko/CommandPool.h"

#include "nstl/unique_ptr.h"

namespace vko
{
    class Queue;
}

namespace gfx_vk
{
    class surface_factory;

    class context
    {
    public:
        context(surface_factory& factory, tglm::ivec2 extent, config const& config);
        ~context();

        VkAllocationCallbacks const& get_allocator() const { return m_allocator; }

        vko::Instance const& get_instance() const;
        vko::Device const& get_device() const;

        VkPhysicalDevice get_physical_device_handle() const;
        VkDevice get_device_handle() const;
        VkSurfaceKHR get_surface_handle() const;

        vko::PhysicalDeviceSurfaceParameters const& get_physical_device_surface_parameters() const;

        vko::Queue const& get_transfer_queue() const;
        vko::CommandPool const& get_transfer_command_pool() const;

        resource_container& get_resources() { return m_resources; }
        resource_container const& get_resources() const { return m_resources; }

        descriptor_allocator& get_descriptor_allocator() { return m_descriptor_allocator; }
        renderer& get_renderer() { return m_renderer; }

        size_t get_mutable_resource_multiplier() const { return m_mutable_resource_multiplier; }
        size_t get_mutable_resource_index() const { return m_mutable_resource_index; }

        void increment_mutable_resource_index() { m_mutable_resource_index = (m_mutable_resource_index + 1) % m_mutable_resource_multiplier; }

        void on_surface_changed();

    private:
        VkAllocationCallbacks m_allocator;

        vko::Instance m_instance;
        vko::Surface m_surface;

        vko::PhysicalDevice m_physical_device;
        vko::PhysicalDeviceSurfaceParameters m_params;

        vko::Device m_device;

        vko::CommandPool m_transfer_command_pool;

        resource_container m_resources;
        descriptor_allocator m_descriptor_allocator;
        renderer m_renderer;

        size_t m_mutable_resource_multiplier = 0;
        size_t m_mutable_resource_index = 0;
    };
}
