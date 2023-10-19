#pragma once

#include <vulkan/vulkan.h>

#include "platform/window.h"

#include "nstl/span.h"

namespace gfx_vk
{
    class surface_factory
    {
    public:
        virtual ~surface_factory() = default;

        virtual nstl::span<char const* const> get_instance_extensions() = 0;
        virtual VkResult create(VkInstance instance, VkSurfaceKHR* handle) = 0;
    };
}
