#pragma once

#include "image.h"
#include "sampler.h"

#include "gfx/resources.h"

namespace gfx_vk
{
    struct texture : gfx::texture
    {
        explicit texture(gfx::texture_params const& params) : image(static_cast<gfx_vk::image*>(params.image)), sampler(static_cast<gfx_vk::sampler*>(params.sampler)) {}

        image* image = nullptr;
        sampler* sampler = nullptr;
    };
}
