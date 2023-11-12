#pragma once

namespace tglm
{
    struct vec2;
    struct vec3;

    float degrees(float rad);
    vec2 degrees(vec2 rad);
    vec3 degrees(vec3 rad);
    float radians(float deg);
    vec2 radians(vec2 deg);
    vec3 radians(vec3 deg);

    float clamp(float v, float min, float max);
}
