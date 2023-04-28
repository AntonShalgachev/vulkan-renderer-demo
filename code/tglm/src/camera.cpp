#include "tglm/camera.h"

#include "tglm/types/mat4.h"

#include "cglm/cam.h"

tglm::mat4 tglm::perspective(float fovy, float aspect, float nearZ, float farZ)
{
    mat4 result;
    glm_perspective(fovy, aspect, nearZ, farZ, result.data);
    return result;
}
