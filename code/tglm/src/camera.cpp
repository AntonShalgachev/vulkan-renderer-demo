#include "tglm/camera.h"

#include "tglm/types/mat4.h"

#include "cglm/cam.h"
#include "cglm/clipspace/persp_lh_zo.h"

tglm::mat4 tglm::perspective_lh_zo(float fovy, float aspect, float nearZ, float farZ)
{
    mat4 result;
    glm_perspective_lh_zo(fovy, aspect, nearZ, farZ, result.data);
    return result;
}
