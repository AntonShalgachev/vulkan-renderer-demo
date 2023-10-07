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

layout(location = 0) in vec3 position;

layout(location = 1) in vec3 in_color;
layout(location = 1) out vec3 out_color;

void main()
{
	gl_Position = frame.projection * frame.view * object.model * vec4(position, 1.0);
    out_color = in_color;
}
