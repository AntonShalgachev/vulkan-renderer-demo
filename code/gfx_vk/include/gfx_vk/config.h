#pragma once

#include <stdint.h>

namespace gfx_vk
{
    struct descriptors_config
    {
        uint32_t max_sets_per_pool = 1024;
        uint32_t max_descriptors_per_type_per_pool = 4 * 1024;
    };

    struct config
    {
        descriptors_config descriptors;
        char const* name = nullptr;
        bool enable_validation;
    };
}
