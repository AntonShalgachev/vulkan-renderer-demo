#include "tglm/types/vec4.h"

#include "tglm/types/vec3.h"

#include "cglm/vec4.h"
#include "cglm/vec3.h"

#include "assert.h"
#include "string.h"

tglm::vec4::vec4(vec3 const& xyz, float w)
{
    glm_vec4(const_cast<vec3&>(xyz).data, w, data);
}

tglm::vec4::vec4(float const* v, [[maybe_unused]] size_t count)
{
    assert(count == elements_count);
    static_assert(elements_count * sizeof(float) == sizeof(data), "");
    memcpy(data, v, sizeof(data));
}

tglm::vec4::operator tglm::vec3() const
{
    vec3 result;
    glm_vec3(const_cast<vec4*>(this)->data, result.data);
    return result;
}

float& tglm::vec4::operator[](size_t index)
{
    assert(index < elements_count);
    return data[index];
}

float const& tglm::vec4::operator[](size_t index) const
{
    assert(index < elements_count);
    return data[index];
}
