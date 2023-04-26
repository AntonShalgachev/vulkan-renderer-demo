#pragma once

#include "tglm/detail/cglm_types.h"

namespace tglm
{
    struct vec3;

    struct vec4
    {
        constexpr static size_t elements_count = 4;

        vec4() : data{} {}
        vec4(float v) : data{ v, v, v, v } {}
        vec4(float x, float y, float z, float w) : data{ x, y, z, w } {}
        vec4(vec3 const& xyz, float w);
        vec4(float const* v, size_t count);
        vec4(float const(&v)[elements_count]) : vec4(v, elements_count) {}
        // TODO vec3 + float constructor

        // TODO other conversion operators?
        operator vec3() const; // TODO move to the vec3's constructors?

        union
        {
            cglm_vec4 data;

            struct
            {
                float x;
                float y;
                float z;
                float w;
            };

            struct
            {
                float r;
                float g;
                float b;
                float a;
            };
        };

        // TODO operators
    };
}

static_assert(sizeof(tglm::vec4) == sizeof(tglm::cglm_vec4), "Unexpected type size");
static_assert(alignof(tglm::vec4) == alignof(tglm::cglm_vec4), "Unexpected type alignment");
static_assert(alignof(tglm::vec4) >= 16, "Unexpected type alignment");
