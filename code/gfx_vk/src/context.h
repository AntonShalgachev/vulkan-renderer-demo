#pragma once

#include "instance.h"
#include "memory.h"
#include "transfers.h"
#include "resource_container.h"
#include "descriptor_allocator.h"
#include "renderer.h"

namespace gfx_vk
{
    class surface_factory;

    class context
    {
    public:
        context(surface_factory& factory, tglm::ivec2 extent, config const& config);
        ~context();

        VkAllocationCallbacks const& get_allocator() const;
        physical_device_properties const& get_physical_device_props() const { return m_instance.get_physical_device_props(); }

        VkPhysicalDevice get_physical_device_handle() const;
        VkSurfaceKHR get_surface_handle() const;
        VkDevice get_device_handle() const;

        // WTF Temporary functions
        void wait_idle() { return m_instance.wait_idle(); }
        uint32_t get_transfer_queue_family_index() const { return m_instance.get_transfer_queue_family_index(); }
        VkQueue get_transfer_queue_handle() const { return m_instance.get_transfer_queue_handle(); }
        uint32_t get_graphics_queue_family_index() const { return m_instance.get_graphics_queue_family_index(); }
        VkQueue get_graphics_queue_handle() const { return m_instance.get_graphics_queue_handle(); }
        VkQueue get_present_queue_handle() const { return m_instance.get_present_queue_handle(); }

        instance& get_instance() { return m_instance; }
        memory& get_memory() { return m_memory; }
        transfers& get_transfers() { return m_transfers; }
        resource_container& get_resources() { return m_resources; }
        descriptor_allocator& get_descriptor_allocator() { return m_descriptor_allocator; }
        renderer& get_renderer() { return m_renderer; }

        size_t get_mutable_resource_multiplier() const { return m_mutable_resource_multiplier; }
        size_t get_mutable_resource_index() const { return m_mutable_resource_index; }

        void increment_mutable_resource_index() { m_mutable_resource_index = (m_mutable_resource_index + 1) % m_mutable_resource_multiplier; }

        void on_surface_changed();

    private:
        instance m_instance;
        memory m_memory;
        transfers m_transfers;
        resource_container m_resources;
        descriptor_allocator m_descriptor_allocator;
        renderer m_renderer;

        size_t m_mutable_resource_multiplier = 0;
        size_t m_mutable_resource_index = 0;
    };
}
