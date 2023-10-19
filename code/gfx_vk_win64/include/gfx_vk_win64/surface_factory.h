#pragma once

#include "gfx_vk/surface_factory.h"

namespace gfx_vk_win64
{
    class surface_factory final : public gfx_vk::surface_factory
    {
    public:
        surface_factory(platform::window_handle_t window);

        nstl::span<char const* const> get_instance_extensions() override;
        VkResult create(VkInstance instance, VkSurfaceKHR* handle) override;

    private:
        platform::window_handle_t m_window = nullptr;
    };
}
