#version 450
#extension GL_ARB_separate_shader_objects : enable

// TODO move to an .h file?
layout(set = 0, binding = 0) uniform FrameViewProjectionData {
    mat4 view;
    mat4 projection;
} frameViewProjection;

layout(set = 1, binding = 0) uniform ObjectUniformBuffer {
    mat4 model;
    vec4 objectColor;
} objectUniforms;

layout(location = 0) in vec3 inPosition;

void main()
{
    mat4 modelView = frameViewProjection.view * objectUniforms.model;
	vec4 viewPos = modelView * vec4(inPosition, 1.0);
	gl_Position = frameViewProjection.projection * viewPos;
}
