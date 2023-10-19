#include "context.h"

#include "gfx_vk/surface_factory.h"

#include "utils.h"

#include "vko/DebugMessage.h"
#include "vko/Queue.h"

#include "nstl/vector.h"

#include "memory/memory.h"
#include "memory/tracking.h"

#include "logging/logging.h"

#include <assert.h>

namespace
{
    static auto scope_id = memory::tracking::create_scope_id("Rendering/Vulkan/Driver");

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

    nstl::vector<const char*> create_instance_extensions(bool enable_validation, gfx_vk::surface_factory& factory)
    {
        nstl::span<const char* const> extensions = factory.get_instance_extensions();

        nstl::vector<const char*> result{ extensions.begin(), extensions.end() };

        if (enable_validation)
            result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return result;
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

    // WTF
    VkSurfaceKHR create_surface(gfx_vk::surface_factory& factory, vko::Instance& instance)
    {
        VkSurfaceKHR handle = VK_NULL_HANDLE;
        GFX_VK_VERIFY(factory.create(instance.getHandle(), &handle));
        return handle;
    }

    void* allocate(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope scope)
    {
        MEMORY_TRACKING_SCOPE(scope_id);

        // TODO make use of alignment
        assert(alignment <= alignof(max_align_t));

        return memory::allocate(size);
    }

    void* reallocate(void* user_data, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope scope)
    {
        MEMORY_TRACKING_SCOPE(scope_id);

        // TODO make use of alignment
        assert(alignment <= alignof(max_align_t));

        return memory::reallocate(ptr, size);
    }

    void deallocate(void* user_data, void* ptr)
    {
        MEMORY_TRACKING_SCOPE(scope_id);

        return memory::deallocate(ptr);
    }

    VkAllocationCallbacks create_allocator()
    {
        return VkAllocationCallbacks{
            .pUserData = nullptr,
            .pfnAllocation = &allocate,
            .pfnReallocation = &reallocate,
            .pfnFree = &deallocate,
        };
    }
}

gfx_vk::context::context(surface_factory& factory, tglm::ivec2 extent, config const& config)
    : m_allocator(create_allocator())
    , m_instance(config.name, create_instance_extensions(config.enable_validation, factory), config.enable_validation, config.enable_validation ? on_debug_message : nullptr)
    , m_surface(create_surface(factory, m_instance), m_instance)
    , m_physical_device(find_physical_device(m_instance, m_surface))
    , m_params(vko::queryPhysicalDeviceSurfaceParameters(m_physical_device, m_surface))
    , m_device(m_physical_device, *m_params.graphicsQueueFamily, *m_params.presentQueueFamily, get_device_extensions())
    , m_transfer_command_pool(m_device.getHandle(), m_device.getGraphicsQueue().getFamily()) // TODO use transfer queue?
    , m_resources(*this)
    , m_descriptor_allocator(*this, config.descriptors)
    , m_renderer(*this, extent, config.renderer)
    , m_mutable_resource_multiplier(config.renderer.max_frames_in_flight)
{

}

gfx_vk::context::~context() = default;

vko::Instance const& gfx_vk::context::get_instance() const
{
    return m_instance;
}

vko::Device const& gfx_vk::context::get_device() const
{
    return m_device;
}

VkPhysicalDevice gfx_vk::context::get_physical_device_handle() const
{
    return m_physical_device.getHandle();
}

VkDevice gfx_vk::context::get_device_handle() const
{
    return m_device.getHandle();
}

VkSurfaceKHR gfx_vk::context::get_surface_handle() const
{
    return m_surface.getHandle();
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
