#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;

layout(push_constant) uniform uPushConstant {
    vec2 scale;
    vec2 translate;
} pcs;

struct Output
{
    vec4 color;
    vec2 uv;
};

layout(location = 0) out Output vertOutput;

void main()
{
    gl_Position = vec4(position * pcs.scale + pcs.translate, 0.0, 1.0);

    vertOutput.color = color;
    vertOutput.uv = uv;
}
