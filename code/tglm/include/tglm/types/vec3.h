#pragma once

#include "tglm/detail/cglm_types.h"

namespace tglm
{
    struct vec3
    {
        constexpr static size_t elements_count = 3;

        vec3() : data{} {}
        vec3(float v) : data{ v, v, v } {}
        vec3(float x, float y, float z) : data{ x, y, z } {}
        vec3(float const* v, size_t count);
        vec3(float const(&v)[elements_count]) : vec3(v, elements_count) {}
        // TODO vec2 + float constructor

        void normalize();
        vec3 normalized() const;

        // TODO conversion operators?

        float& operator[](size_t index);
        float const& operator[](size_t index) const;

        union
        {
            cglm_vec3 data;

            struct
            {
                float x;
                float y;
                float z;
            };

            struct
            {
                float r;
                float g;
                float b;
            };
        };
    };

    // TODO other operators
    vec3 operator*(vec3 const& lhs, float rhs);
    inline vec3 operator*(float lhs, vec3 const& rhs) { return rhs * lhs; }

    vec3& operator+=(vec3& lhs, vec3 const& rhs);

    vec3 operator-(vec3 const& v);
}

static_assert(sizeof(tglm::vec3) == sizeof(tglm::cglm_vec3), "Unexpected type size");
static_assert(alignof(tglm::vec3) == alignof(tglm::cglm_vec3), "Unexpected type alignment");
