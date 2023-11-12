#pragma once

#include "tglm/detail/cglm_types.h"

namespace tglm
{
    struct vec3;
    struct mat4;

    struct quat
    {
        constexpr static size_t elements_count = 4;

        static quat identity();
        static quat from_euler_xyz(vec3 const& angles);
        static quat from_euler_zyx(vec3 const& angles);
        static quat from_axis_rotation(float angle, vec3 const& axis);

        quat() : data{} {}
        quat(float x, float y, float z, float w) : data{ x, y, z, w } {}
        quat(float const* v, size_t count);
        quat(float const(&v)[elements_count]) : quat(v, elements_count) {}

        vec3 rotate(vec3 const& v) const;

        vec3 to_euler_xyz() const;
//         mat3 to_mat3() const; // TODO
        mat4 to_mat4() const;

        union
        {
            cglm_versor data;

            struct
            {
                float x;
                float y;
                float z;
                float w;
            };
        };
    };

    // TODO other operators
    quat operator*(quat const& lhs, quat const& rhs);
    quat& operator*=(quat& lhs, quat const& rhs);

    vec3 operator*(quat const& lhs, vec3 const& rhs);
}

static_assert(sizeof(tglm::quat) == sizeof(tglm::cglm_versor), "Unexpected type size");
static_assert(alignof(tglm::quat) == alignof(tglm::cglm_versor), "Unexpected type alignment");
static_assert(alignof(tglm::quat) >= 16, "Unexpected type alignment");
