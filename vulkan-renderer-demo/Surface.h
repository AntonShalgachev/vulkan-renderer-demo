#pragma once

#include "framework.h"

namespace vkr
{
    class Instance;

    class Surface
    {
    public:
        explicit Surface(Instance const& instance, GLFWwindow* window);
        ~Surface();

        Surface(Surface const&) = delete;
        Surface(Surface&&);
        Surface& operator=(Surface const&) = delete;
        Surface& operator=(Surface&&) = delete;

        void onSurfaceChanged();

        VkSurfaceKHR const& getHandle() const { return m_handle; }

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

    private:
        VkSurfaceKHR m_handle;

        Instance const& m_instance;
        GLFWwindow* m_window;

        int m_width = -1;
        int m_height = -1;
    };
}
