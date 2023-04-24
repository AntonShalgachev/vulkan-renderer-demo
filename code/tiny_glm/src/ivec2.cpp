#include "tiny_glm/types/ivec2.h"

#include "cglm/ivec2.h"

#include "assert.h"
#include "string.h"

tglm::ivec2::ivec2(int const* v, [[maybe_unused]] size_t count)
{
    assert(count == elements_count);
    static_assert(elements_count * sizeof(int) == sizeof(data), "");
    memcpy(data, v, sizeof(data));
}

tglm::ivec2 tglm::operator*(ivec2 const& lhs, int rhs)
{
    ivec2 result;
    glm_ivec2_scale(const_cast<ivec2&>(lhs).data, rhs, result.data);
    return result;
}