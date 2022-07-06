#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 modelView;
    mat4 normal;
    mat4 projection;
    vec3 lightPosition;
    vec3 viewPos;
    vec4 objectColor;
} ubo;

layout(location = 0) in vec3 inPosition;

#ifdef HAS_VERTEX_COLOR
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;
#endif

#ifdef HAS_TEX_COORD
layout(location = 2) in vec2 inTexCoord;
layout(location = 1) out vec2 fragTexCoord;
#endif

#ifdef HAS_NORMAL
layout(location = 3) in vec3 inNormal;
layout(location = 2) out vec3 fragNormal;
#endif

#ifdef HAS_TANGENT
layout(location = 4) in vec3 inTangent;
layout(location = 3) out vec3 fragTangent;
#endif

layout(location = 4) out vec3 viewVec;
layout(location = 5) out vec3 lightVec;

layout(location = 6) out vec4 objectColor;

void main()
{
	vec4 viewPos = ubo.modelView * vec4(inPosition, 1.0);
	gl_Position = ubo.projection * viewPos;

#ifdef HAS_VERTEX_COLOR
    fragColor = inColor;
#endif
#ifdef HAS_TEX_COORD
    fragTexCoord = inTexCoord;
#endif
#ifdef HAS_NORMAL
    fragNormal = (ubo.normal * vec4(inNormal, 0.0)).xyz;
#endif
#ifdef HAS_TANGENT
    fragTangent = inTangent;
#endif

    objectColor = ubo.objectColor;

	lightVec = ubo.lightPosition - viewPos.xyz;
	viewVec = viewPos.xyz - ubo.viewPos;
}
