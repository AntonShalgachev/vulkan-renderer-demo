#include "tglm/affine.h"

#include "tglm/types/vec3.h"
#include "tglm/types/vec4.h"
#include "tglm/types/mat4.h"
#include "tglm/types/quat.h"

#include "cglm/affine.h"
#include "cglm/quat.h" // TODO why

void tglm::translate(mat4& m, vec3 const& v)
{
    glm_translate(m.data, const_cast<vec3&>(v).data);
}

tglm::mat4 tglm::translated(mat4 const& m, vec3 const& v)
{
    mat4 result;
    glm_translate_to(const_cast<mat4&>(m).data, const_cast<vec3&>(v).data, result.data);
    return result;
}

void tglm::rotate(mat4& m, quat const& q)
{
    m *= q.to_mat4(); // TODO use glm_quat_rotate
}

tglm::mat4 tglm::rotated(mat4 const& m, quat const& q)
{
    mat4 result;
    glm_quat_rotate(const_cast<mat4&>(m).data, const_cast<quat&>(q).data, result.data);
    return result;
}

void tglm::scale(mat4& m, vec3 const& s)
{
    glm_scale(m.data, const_cast<vec3&>(s).data);
}

tglm::mat4 tglm::scaled(mat4 const& m, vec3 const& s)
{
    mat4 result;
    glm_scale_to(const_cast<mat4&>(m).data, const_cast<vec3&>(s).data, result.data);
    return result;
}

void tglm::decompose(mat4 const& m, vec4& translation, mat4& rotation, vec3& scale)
{
    glm_decompose(const_cast<mat4&>(m).data, translation.data, rotation.data, scale.data);
}

void tglm::decompose(mat4 const& m, vec4& translation, quat& rotation, vec3& scale)
{
    mat4 rotation_matrix;
    decompose(m, translation, rotation_matrix, scale);

    rotation = rotation_matrix.to_quat();
}
