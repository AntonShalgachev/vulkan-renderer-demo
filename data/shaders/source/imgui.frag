#version 450 core

layout(set = 1, binding = 0) uniform sampler2D albedo;

struct Input
{
    vec4 color;
    vec2 uv;
};

layout(location = 0) in Input fragInput;

layout(location = 0) out vec4 fragOutput;

void main()
{
    fragOutput = fragInput.color * texture(albedo, fragInput.uv.st);
}