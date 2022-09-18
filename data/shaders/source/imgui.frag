#version 450 core

layout(set = 0, binding = 1) uniform sampler2D albedo;

struct Input
{
    vec4 color;
    vec2 uv;
};

layout(location = 0) in Input fragInput;

layout(location = 0) out vec4 fragOutput;

void main()
{
    // fragOutput = vec4(1.0, 0.0, 0.0, 0.0);
    fragOutput = fragInput.color * texture(albedo, fragInput.uv.st);
}