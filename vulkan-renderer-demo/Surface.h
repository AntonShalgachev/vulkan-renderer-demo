#pragma once

#include <vulkan/vulkan.h>

namespace vkr
{
    class Instance;
    class Window;

    class Surface
    {
    public:
        explicit Surface(Instance const& instance, Window const& window);
        ~Surface();

        Surface(Surface const&) = delete;
        Surface(Surface&&);
        Surface& operator=(Surface const&) = delete;
        Surface& operator=(Surface&&) = delete;

        VkSurfaceKHR const& getHandle() const { return m_handle; }

        int getWidth() const;
        int getHeight() const;

    private:
        VkSurfaceKHR m_handle;

        Instance const& m_instance;
        Window const& m_window;
    };
}
