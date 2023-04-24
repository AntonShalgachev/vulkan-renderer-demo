#pragma once

#include "tiny_glm/detail/cglm_types.h"

namespace tglm
{
    struct vec2
    {
        constexpr static size_t elements_count = 2;

        vec2() : data{} {}
        vec2(float v) : data{ v, v } {}
        vec2(float x, float y) : data{ x, y } {}
        vec2(float const* v, size_t count);
        vec2(float const(&v)[elements_count]) : vec2(v, elements_count) {}

        void normalize();
        vec2 normalized() const;

        // TODO conversion operators?

        union
        {
            cglm_vec2 data;

            struct
            {
                float x;
                float y;
            };

            struct
            {
                float u;
                float v;
            };
        };
    };

    // TODO other operators
    vec2 operator*(vec2 const& lhs, float rhs);
    inline vec2 operator*(float lhs, vec2 const& rhs) { return rhs * lhs; }
}

static_assert(sizeof(tglm::vec2) == sizeof(tglm::cglm_vec2), "Unexpected type size");
static_assert(alignof(tglm::vec2) == alignof(tglm::cglm_vec2), "Unexpected type alignment");
