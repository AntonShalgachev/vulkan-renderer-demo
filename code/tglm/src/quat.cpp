#include "tglm/types/quat.h"

#include "tglm/types/vec3.h"
#include "tglm/types/mat4.h"

#include "cglm/quat.h"
#include "cglm/euler.h"

#include "assert.h"
#include "string.h"

tglm::quat tglm::quat::identity()
{
    tglm::quat result;
    glm_quat_identity(result.data);
    return result;
}

tglm::quat tglm::quat::from_euler_xyz(vec3 const& angles)
{
    // TODO make it more optimal when cglm allows it

    cglm_mat4 m;
    glm_euler_xyz(const_cast<vec3&>(angles).data, m);

    tglm::quat result;
    glm_mat4_quat(m, result.data);

    return result;
}

tglm::quat tglm::quat::from_euler_zyx(vec3 const& angles)
{
    // TODO make it more optimal when cglm allows it

    cglm_mat4 m;
    glm_euler_zyx(const_cast<vec3&>(angles).data, m);

    tglm::quat result;
    glm_mat4_quat(m, result.data);

    return result;
}

tglm::quat::quat(float const* v, [[maybe_unused]] size_t count)
{
    assert(count == elements_count);
    static_assert(elements_count * sizeof(float) == sizeof(data), "");
    memcpy(data, v, sizeof(data));
}

tglm::vec3 tglm::quat::rotate(vec3 const& v) const
{
    vec3 result;
    glm_quat_rotatev(const_cast<quat*>(this)->data, const_cast<vec3&>(v).data, result.data);
    return result;
}

tglm::vec3 tglm::quat::to_euler_xyz() const
{
    // TODO make it more optimal when cglm allows it

    mat4 m = to_mat4();

    tglm::vec3 result;
    glm_euler_angles(m.data, result.data);

    return result;
}

// tglm::mat3 tglm::quat::to_mat3() const
// {
//     mat3 result;
//     glm_quat_mat3(const_cast<quat*>(this)->data, result.data);
//     return result;
// }

tglm::mat4 tglm::quat::to_mat4() const
{
    mat4 result;
    glm_quat_mat4(const_cast<quat*>(this)->data, result.data);
    return result;
}

// tglm::quat& tglm::operator*=(quat const& lhs, quat const& rhs)
// {
// //     glm_quat_mul()
// }

tglm::quat tglm::operator*(quat const& lhs, quat const& rhs)
{
    quat result;
    glm_quat_mul(const_cast<quat&>(lhs).data, const_cast<quat&>(rhs).data, result.data);
    return result;
}

tglm::quat& tglm::operator*=(quat& lhs, quat const& rhs)
{
    glm_quat_mul(lhs.data, const_cast<quat&>(rhs).data, lhs.data);
    return lhs;
}

tglm::vec3 tglm::operator*(quat const& lhs, vec3 const& rhs)
{
    vec3 result;
    glm_quat_rotatev(const_cast<quat&>(lhs).data, const_cast<vec3&>(rhs).data, result.data);
    return result;
}
