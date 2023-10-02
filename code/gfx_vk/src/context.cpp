#include "context.h"

#include "vko/DebugMessage.h"
#include "vko/Queue.h"

#include "nstl/vector.h"

#include "logging/logging.h"

#include <assert.h>

namespace
{
    nstl::span<char const* const> get_device_extensions()
    {
        static nstl::vector<char const*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        return extensions;
    }

    bool is_device_suitable(vko::PhysicalDevice const& physical_device, vko::Surface const& surface)
    {
        if (!physical_device.areExtensionsSupported(get_device_extensions()))
            return false;

        if (!physical_device.getFeatures().samplerAnisotropy)
            return false;

        vko::PhysicalDeviceSurfaceParameters params = vko::queryPhysicalDeviceSurfaceParameters(physical_device, surface);
        return params.graphicsQueueFamily && params.presentQueueFamily && !params.formats.empty() && !params.presentModes.empty();
    }

    nstl::vector<const char*> create_instance_extensions(bool enable_validation, vko::Window const& window)
    {
        nstl::vector<const char*> extensions = window.getRequiredInstanceExtensions();

        if (enable_validation)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    vko::PhysicalDevice find_physical_device(vko::Instance const& instance, vko::Surface const& surface)
    {
        nstl::vector<vko::PhysicalDevice> physical_devices = instance.findPhysicalDevices();

        for (vko::PhysicalDevice& physical_device : physical_devices)
            if (is_device_suitable(physical_device, surface))
                return nstl::move(physical_device);

        assert(false);
        return nstl::move(physical_devices[0]);
    }

    void on_debug_message(vko::DebugMessage const& message)
    {
        // TODO don't log "Info" level to the console
		if (message.level == vko::DebugMessage::Level::Info)
			logging::info("{}", message.text);
        if (message.level == vko::DebugMessage::Level::Warning)
            logging::warn("{}", message.text);
        if (message.level == vko::DebugMessage::Level::Error)
            logging::error("{}", message.text);

        assert(message.level != vko::DebugMessage::Level::Error);
    }
}

gfx_vk::context::context(vko::Window const& window, char const* name, bool enable_validation)
    : m_instance(name, create_instance_extensions(enable_validation, window), enable_validation, enable_validation ? on_debug_message : nullptr)
    , m_surface(window.createSurface(m_instance))
    , m_physical_device(find_physical_device(m_instance, m_surface))
    , m_params(vko::queryPhysicalDeviceSurfaceParameters(m_physical_device, m_surface))
    , m_device(m_physical_device, *m_params.graphicsQueueFamily, *m_params.presentQueueFamily, get_device_extensions())
    , m_transfer_command_pool(m_device, m_device.getGraphicsQueue().getFamily()) // TODO use transfer queue?
    , m_resources(*this)
{

}

gfx_vk::context::~context() = default;

vko::Instance const& gfx_vk::context::get_instance() const
{
    return m_instance;
}

vko::Surface const& gfx_vk::context::get_surface() const
{
    return m_surface;
}

vko::Device const& gfx_vk::context::get_device() const
{
    return m_device;
}

vko::PhysicalDevice const& gfx_vk::context::get_physical_device() const
{
    return m_physical_device;
}

vko::PhysicalDeviceSurfaceParameters const& gfx_vk::context::get_physical_device_surface_parameters() const
{
    return m_params;
}

vko::Queue const& gfx_vk::context::get_transfer_queue() const
{
    return m_device.getGraphicsQueue(); // TODO use transfer queue?
}

vko::CommandPool const& gfx_vk::context::get_transfer_command_pool() const
{
    return m_transfer_command_pool;
}

void gfx_vk::context::on_surface_changed()
{
    m_params = vko::queryPhysicalDeviceSurfaceParameters(m_physical_device, m_surface);
}
