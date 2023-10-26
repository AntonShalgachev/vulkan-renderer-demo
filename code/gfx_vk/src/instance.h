#pragma once

#include "utils.h"

#include "common/fmt.h"

#include "nstl/unique_ptr.h"

namespace gfx_vk
{
    struct config;
    class surface_factory;

    struct physical_device_properties
    {
        VkPhysicalDeviceProperties properties{};

        VkSurfaceCapabilitiesKHR capabilities{};
        nstl::vector<VkSurfaceFormatKHR> formats;
        nstl::vector<VkPresentModeKHR> present_modes;
        nstl::optional<uint32_t> transfer_queue_family;
        nstl::optional<uint32_t> graphics_queue_family;
        nstl::optional<uint32_t> present_queue_family;

        VkPhysicalDeviceMemoryProperties memory_properties;
    };

    class instance
    {
    public:
        instance(surface_factory& factory, config const& config);
        ~instance();

        VkAllocationCallbacks const& get_allocator() const { return m_allocator; }
        physical_device_properties const& get_physical_device_props() const { return m_physical_device_props; }

        VkPhysicalDevice get_physical_device_handle() const { return m_physical_device; }
        VkSurfaceKHR get_surface_handle() const { return m_surface; }
        VkDevice get_device_handle() const { return m_device; }

        void wait_idle();
        uint32_t get_transfer_queue_family_index() const { return *m_physical_device_props.transfer_queue_family; }
        VkQueue get_transfer_queue_handle() const { return m_transfer_queue; }
        uint32_t get_graphics_queue_family_index() const { return *m_physical_device_props.graphics_queue_family; }
        VkQueue get_graphics_queue_handle() const { return m_graphics_queue; }
        VkQueue get_present_queue_handle() const { return m_present_queue; }

        void on_surface_changed();

        template<vulkan_handle T, picofmt::formattable... Ts>
        void set_debug_name(unique_handle<T> const& handle, nstl::string_view format, Ts const&... args)
        {
            return set_debug_name_v(reinterpret_cast<uint64_t>(handle.get()), object_type_v<T>, format, picofmt::args_list{ args... });
        }

        template<vulkan_handle T, picofmt::formattable... Ts>
        void set_debug_name(T handle, nstl::string_view format, Ts const&... args)
        {
            return set_debug_name_v(reinterpret_cast<uint64_t>(handle), object_type_v<T>, format, picofmt::args_list{ args... });
        }

    private:
        void create_instance(surface_factory& factory, config const& config);
        void load_debug_functions(config const& config);
        void setup_debug_callback(config const& config);
        void find_physical_device();
        void create_device();

        void set_debug_name_v(uint64_t handle, VkObjectType type, nstl::string_view format, picofmt::args_list const& args);

    private:
        VkAllocationCallbacks m_allocator{};

        unique_handle<VkInstance> m_instance;
        PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessengerEXT = nullptr;
        PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT = nullptr;
        unique_handle<VkDebugUtilsMessengerEXT> m_debug_messenger;

        unique_handle<VkSurfaceKHR> m_surface;

        VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
        physical_device_properties m_physical_device_props{};

        unique_handle<VkDevice> m_device;
        VkQueue m_transfer_queue = VK_NULL_HANDLE;
        VkQueue m_graphics_queue = VK_NULL_HANDLE;
        VkQueue m_present_queue = VK_NULL_HANDLE;
    };
}
