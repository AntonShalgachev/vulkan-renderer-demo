#include "context.h"

gfx_vk::context::context(surface_factory& factory, size_t w, size_t h, config const& config)
    : m_instance(factory, config)
    , m_memory(*this)
    , m_transfers(*this)
    , m_resources(*this)
    , m_descriptor_allocator(*this, config.descriptors)
    , m_renderer(*this, w, h, config.renderer)
    , m_mutable_resource_multiplier(config.renderer.max_frames_in_flight)
{

}

gfx_vk::context::~context() = default;

VkAllocationCallbacks const& gfx_vk::context::get_allocator() const
{
    return m_instance.get_allocator();
}

VkPhysicalDevice gfx_vk::context::get_physical_device_handle() const
{
    return m_instance.get_physical_device_handle();
}

VkDevice gfx_vk::context::get_device_handle() const
{
    return m_instance.get_device_handle();
}

VkSurfaceKHR gfx_vk::context::get_surface_handle() const
{
    return m_instance.get_surface_handle();
}

void gfx_vk::context::on_surface_changed()
{
    return m_instance.on_surface_changed();
}
