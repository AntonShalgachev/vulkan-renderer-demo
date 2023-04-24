#include "tiny_glm/affine.h"

#include "tiny_glm/types/vec3.h"
#include "tiny_glm/types/vec4.h"
#include "tiny_glm/types/mat4.h"
#include "tiny_glm/types/quat.h"

#include "cglm/affine.h"

void tglm::translate(mat4& m, vec3 const& v)
{
    glm_translate(m.data, const_cast<vec3&>(v).data);
}

void tglm::rotate(mat4& m, quat const& q)
{
    m *= q.to_mat4(); // TODO use glm_quat_rotate
}

void tglm::scale(mat4& m, vec3 const& s)
{
    glm_scale(m.data, const_cast<vec3&>(s).data);
}

void tglm::decompose(mat4 const& m, vec4& translation, mat4& rotation, vec3& scale)
{
    glm_decompose(const_cast<mat4&>(m).data, translation.data, rotation.data, scale.data);
}

void tglm::decompose(mat4 const& m, vec4& translation, quat& rotation, vec3& scale)
{
    mat4 rotation_matrix;
    decompose(m, translation, rotation_matrix, scale);

    rotation = quat{ rotation_matrix };
}
