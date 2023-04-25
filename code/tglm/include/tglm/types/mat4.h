#pragma once

#include "tglm/detail/cglm_types.h"

namespace tglm
{
    struct mat4
    {
        constexpr static size_t elements_count = 16;

        static mat4 identity();

        mat4() : data{} {}
        mat4(float const* v, size_t count);
        mat4(float const(&v)[elements_count]) : mat4(v, elements_count) {}

        union
        {
            cglm_mat4 data;

            // TODO add more representations
        };
    };

    // TODO other operators
    mat4 operator*(mat4 const& lhs, mat4 const& rhs);
    mat4& operator*=(mat4& lhs, mat4 const& rhs);
}

static_assert(sizeof(tglm::mat4) == sizeof(tglm::cglm_mat4), "Unexpected type size");
static_assert(alignof(tglm::mat4) == alignof(tglm::cglm_mat4), "Unexpected type alignment");
static_assert(alignof(tglm::mat4) >= 16, "Unexpected type alignment");
