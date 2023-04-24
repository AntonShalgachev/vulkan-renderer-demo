#include "tglm/util.h"

#include "tglm/types/vec3.h"

tglm::vec3 tglm::degrees(vec3 rad)
{
    return rad * (GLM_PIf / 180.0f);
}

tglm::vec3 tglm::radians(vec3 deg)
{
    return deg * (180.0f / GLM_PIf);
}
