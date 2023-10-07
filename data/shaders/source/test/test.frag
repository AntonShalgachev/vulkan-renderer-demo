#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform FrameUniformBuffer {
    mat4 view;
    mat4 projection;
} frame;

layout(set = 1, binding = 0) uniform MaterialUniformBuffer {
    vec4 color;
} material;

layout(set = 2, binding = 0) uniform ObjectUniformBuffer {
    mat4 model;
    vec4 color;
} object;

layout(location = 1) in vec3 in_color;

layout(location = 0) out vec4 color_out;

void main()
{
	color_out = vec4(in_color, 1.0) * material.color * object.color;
}
