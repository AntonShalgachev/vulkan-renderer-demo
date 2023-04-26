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

void tglm::vec3::normalize()
{
    glm_vec3_normalize(data);
}

tglm::vec3 tglm::vec3::normalized() const
{
    vec3 result;
    glm_vec3_normalize_to(const_cast<vec3*>(this)->data, result.data);
    return result;
}

float& tglm::vec3::operator[](size_t index)
{
    assert(index < elements_count);
    return data[index];
}

float const& tglm::vec3::operator[](size_t index) const
{
    assert(index < elements_count);
    return data[index];
}

tglm::vec3 tglm::operator*(vec3 const& lhs, float rhs)
{
    vec3 result;
    glm_vec3_scale(const_cast<vec3&>(lhs).data, rhs, result.data);
    return result;
}

tglm::vec3& tglm::operator+=(vec3& lhs, vec3 const& rhs)
{
    glm_vec3_add(lhs.data, const_cast<vec3&>(rhs).data, lhs.data);
    return lhs;
}

tglm::vec3 tglm::operator-(vec3 const& v)
{
    vec3 result;
    glm_vec3_negate_to(const_cast<vec3&>(v).data, result.data);
    return result;
}
