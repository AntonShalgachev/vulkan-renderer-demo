#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;

layout(set = 0, binding = 0) uniform TransformParams {
    vec2 scale;
    vec2 translate;
} transform;

struct Output
{
    vec4 color;
    vec2 uv;
};

layout(location = 0) out Output vertOutput;

void main()
{
    gl_Position = vec4(position * transform.scale + transform.translate, 0.0, 1.0);

    vertOutput.color = color;
    vertOutput.uv = uv;
}
