#version 450
#extension GL_ARB_separate_shader_objects : enable

// TODO move to an .h file?
layout(set = 0, binding = 0) uniform FrameUniformBuffer {
    mat4 view;
    mat4 projection;
    vec3 lightPosition;
    vec3 lightColor;
} frameUniforms;

layout(push_constant) uniform PushConstants {
    mat4 modelViewProjection;
} pcs;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

void main()
{
	gl_Position = pcs.modelViewProjection * vec4(inPosition, 1.0);
}
