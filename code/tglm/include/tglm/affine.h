#pragma once

namespace tglm
{
    struct vec3;
    struct vec4;
    struct mat4;
    struct quat;

    // TODO post-transformations?
    // TODO out-of-place transformation

    void translate(mat4& m, vec3 const& v);
    mat4 translated(mat4 const& m, vec3 const& v);
    void rotate(mat4& m, quat const& q);
    mat4 rotated(mat4 const& m, quat const& q);
    void scale(mat4& m, vec3 const& s);
    mat4 scaled(mat4 const& m, vec3 const& s);

    void decompose(mat4 const& m, vec4& translation, mat4& rotation, vec3& scale);
    void decompose(mat4 const& m, vec4& translation, quat& rotation, vec3& scale);
}
