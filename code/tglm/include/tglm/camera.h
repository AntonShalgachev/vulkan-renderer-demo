#pragma once

namespace tglm
{
    struct mat4;

    // TODO implement other options
    // left-handed, zero-to-one
    mat4 perspective_lh_zo(float fovy, float aspect, float nearZ, float farZ);
}
