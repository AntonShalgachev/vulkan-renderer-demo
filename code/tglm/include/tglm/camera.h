#pragma once

namespace tglm
{
    struct mat4;

    // TODO implement options (left-handed/right-handed, [0;1]/[-1;1])
    mat4 perspective(float fovy, float aspect, float nearZ, float farZ);
}
