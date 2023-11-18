#include "tglm/types/ivec2.h"

#include "tglm/types/vec2.h"

#include "cglm/ivec2.h"

#include "assert.h"
#include "string.h"

tglm::ivec2::ivec2(int const* v, [[maybe_unused]] size_t count)
{
    assert(count == elements_count);
    static_assert(elements_count * sizeof(int) == sizeof(data), "");
    memcpy(data, v, sizeof(data));
}

tglm::ivec2::operator tglm::vec2() const
{
    return { static_cast<float>(data[0]), static_cast<float>(data[1]) };
}

tglm::ivec2 tglm::operator-(ivec2 const& lhs, ivec2 const& rhs)
{
    ivec2 result;
    glm_ivec2_sub(const_cast<ivec2&>(lhs).data, const_cast<ivec2&>(rhs).data, result.data);
    return result;
}

tglm::ivec2 tglm::operator*(ivec2 const& lhs, int rhs)
{
    ivec2 result;
    glm_ivec2_scale(const_cast<ivec2&>(lhs).data, rhs, result.data);
    return result;
}
