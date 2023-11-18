#pragma once

#include "tglm/detail/cglm_types.h"

namespace tglm
{
    struct vec2;

    struct ivec2
    {
        constexpr static size_t elements_count = 2;

        ivec2() : data{} {}
        ivec2(int v) : data{ v, v } {}
        ivec2(int x, int y) : data{ x, y } {}
        ivec2(int const* v, size_t count);
        ivec2(int const(&v)[elements_count]) : ivec2(v, elements_count) {}

        // TODO conversion operators?
        explicit operator vec2() const;

        union
        {
            cglm_ivec2 data;

            struct
            {
                int x;
                int y;
            };

            struct
            {
                int u;
                int v;
            };
        };
    };

    // TODO operators
    ivec2 operator-(ivec2 const& lhs, ivec2 const& rhs);
    ivec2 operator*(ivec2 const& lhs, int rhs);
    inline ivec2 operator*(int lhs, ivec2 const& rhs) { return rhs * lhs; }
}

static_assert(sizeof(tglm::ivec2) == sizeof(tglm::cglm_ivec2), "Unexpected type size");
static_assert(alignof(tglm::ivec2) == alignof(tglm::cglm_ivec2), "Unexpected type alignment");
