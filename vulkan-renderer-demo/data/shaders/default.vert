#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 modelView;
    mat4 normal;
    mat4 projection;
    vec3 lightPosition;
    vec3 viewPos;
} ubo;

layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

// layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 viewVec;
layout(location = 4) out vec3 lightVec;

void main()
{
	vec4 viewPos = ubo.modelView * vec4(inPosition, 1.0);

	gl_Position = ubo.projection * viewPos;
    // fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = (ubo.normal * vec4(inNormal, 0.0)).xyz;

	lightVec = ubo.lightPosition - viewPos.xyz;
	viewVec = viewPos.xyz - ubo.viewPos;
}
