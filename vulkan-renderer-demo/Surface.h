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

        VkSurfaceKHR const& getHandle() const { return m_handle; }

    private:
        VkSurfaceKHR m_handle;

        Instance const& m_instance;
    };
}
