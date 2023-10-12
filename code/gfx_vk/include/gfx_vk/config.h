#pragma once

#include <stdint.h>

namespace gfx_vk
{
    struct descriptors_config
    {
        uint32_t max_sets_per_pool = 1024;
        uint32_t max_descriptors_per_type_per_pool = 4 * 1024;
    };

    struct renderer_config
    {
        size_t max_frames_in_flight = 3; // also the mutable resource multiplier
    };

    struct config
    {
        descriptors_config descriptors;
        renderer_config renderer;
        char const* name = nullptr;
        bool enable_validation;
    };
}
