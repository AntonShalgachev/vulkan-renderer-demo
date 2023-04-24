#include "tglm/types/vec3.h"

#include "cglm/vec3.h"

#include "assert.h"
#include "string.h"

tglm::vec3::vec3(float const* v, [[maybe_unused]] size_t count)
{
    assert(count == elements_count);
    static_assert(elements_count * sizeof(float) == sizeof(data), "");
    memcpy(data, v, sizeof(data));
}

tglm::vec3 tglm::operator*(vec3 const& lhs, float rhs)
{
    vec3 result;
    glm_vec3_scale(const_cast<vec3&>(lhs).data, rhs, result.data);
    return result;
}
