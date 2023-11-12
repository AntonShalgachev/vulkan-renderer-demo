#include "tglm/util.h"

#include "tglm/types/vec2.h"
#include "tglm/types/vec3.h"

#include "cglm/util.h"

float tglm::degrees(float rad)
{
    return rad * (180.0f / GLM_PIf);
}

tglm::vec2 tglm::degrees(vec2 rad)
{
    return rad * (180.0f / GLM_PIf);
}

tglm::vec3 tglm::degrees(vec3 rad)
{
    return rad * (180.0f / GLM_PIf);
}

float tglm::radians(float deg)
{
    return deg * (GLM_PIf / 180.0f);
}

tglm::vec2 tglm::radians(vec2 deg)
{
    return deg * (GLM_PIf / 180.0f);
}

tglm::vec3 tglm::radians(vec3 deg)
{
    return deg * (GLM_PIf / 180.0f);
}

float tglm::clamp(float v, float min, float max)
{
    return glm_clamp(v, min, max);
}
