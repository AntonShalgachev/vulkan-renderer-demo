#version 450
#extension GL_ARB_separate_shader_objects : enable

// TODO move to an .h file?
layout(set = 0, binding = 0) uniform FrameViewProjectionData {
    mat4 view;
    mat4 projection;
} frameViewProjection;
layout(set = 0, binding = 1) uniform FrameLightData {
    mat4 lightViewProjection;
    vec3 lightPosition;
    vec3 lightColor;
} frameLight;

layout(set = 1, binding = 0) readonly buffer ObjectUniformBuffer {
    mat4 model[];
} objectUniforms;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

void main()
{
	gl_Position = frameViewProjection.projection * frameViewProjection.view * objectUniforms.model[gl_InstanceIndex] * vec4(inPosition, 1.0);
}
