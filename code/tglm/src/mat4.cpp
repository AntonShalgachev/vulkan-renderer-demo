#include "tglm/types/mat4.h"

#include "cglm/mat4.h"

#include "assert.h"
#include "string.h"

tglm::mat4 tglm::mat4::identity()
{
    mat4 result;
    glm_mat4_identity(result.data);
    return result;
}

tglm::mat4::mat4(float const* v, [[maybe_unused]] size_t count)
{
    assert(count == elements_count);
    static_assert(elements_count * sizeof(float) == sizeof(data), "");
    memcpy(data, v, sizeof(data));
}

tglm::mat4 tglm::operator*(mat4 const& lhs, mat4 const& rhs)
{
    mat4 result;
    glm_mat4_mul(const_cast<mat4&>(lhs).data, const_cast<mat4&>(rhs).data, result.data);
    return result;
}

tglm::mat4& tglm::operator*=(mat4& lhs, mat4 const& rhs)
{
    glm_mat4_mul(lhs.data, const_cast<mat4&>(rhs).data, lhs.data);
    return lhs;
}
