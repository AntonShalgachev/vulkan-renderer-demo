#version 450
#extension GL_ARB_separate_shader_objects : enable

// TODO move to an .h file?
layout(set = 0, binding = 0) uniform FrameUniformBuffer {
    mat4 view;
    mat4 projection;
} frameUniforms;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pcs;

layout(location = 0) in vec3 inPosition;

void main()
{
    mat4 modelView = frameUniforms.view * pcs.model;
	vec4 viewPos = modelView * vec4(inPosition, 1.0);
	gl_Position = frameUniforms.projection * viewPos;
}
