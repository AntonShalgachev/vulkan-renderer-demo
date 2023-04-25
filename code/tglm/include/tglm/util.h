#pragma once

namespace tglm
{
    struct vec3;

    vec3 degrees(vec3 rad);
    vec3 radians(vec3 deg);

    float clamp(float v, float min, float max);
}
