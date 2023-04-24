#include "tglm/types/vec2.h"

#include "cglm/vec2.h"

#include "assert.h"
#include "string.h"

tglm::vec2::vec2(float const* v, [[maybe_unused]] size_t count)
{
    assert(count == elements_count);
    static_assert(elements_count * sizeof(float) == sizeof(data), "");
    memcpy(data, v, sizeof(data));
}

tglm::vec2 tglm::operator*(vec2 const& lhs, float rhs)
{
    vec2 result;
    glm_vec2_scale(const_cast<vec2&>(lhs).data, rhs, result.data);
    return result;
}

void tglm::vec2::normalize()
{
    glm_vec2_normalize(data);
}

tglm::vec2 tglm::vec2::normalized() const
{
    vec2 result;
    glm_vec2_normalize_to(const_cast<vec2*>(this)->data, result.data);
    return result;
}
