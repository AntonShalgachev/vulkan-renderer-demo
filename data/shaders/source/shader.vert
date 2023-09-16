#version 450
#extension GL_ARB_separate_shader_objects : enable

#if defined(HAS_NORMAL) && defined(HAS_TANGENT)
#define HAS_BITANGENT
#endif

// TODO move to an .h file?
layout(set = 0, binding = 0) uniform FrameUniformBuffer {
    mat4 view;
    mat4 projection;
    mat4 lightViewProjection;
    vec3 lightPosition;
    vec3 lightColor;
} frameUniforms;

layout(set = 1, binding = 0) uniform MaterialUniformBuffer {
    vec4 objectColor;
} materialUniforms;

layout(set = 2, binding = 0) uniform ObjectUniformBuffer {
    vec4 objectColor;
} objectUniforms;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pcs;

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
layout(location = 4) in vec4 inTangent;
layout(location = 3) out vec3 fragTangent;
#endif

layout(location = 4) out vec3 viewVec;
layout(location = 5) out vec3 lightVec;
layout(location = 6) out vec3 lightColor; // TODO should be a uniform

layout(location = 7) out vec4 objectColor;

#ifdef HAS_BITANGENT
layout(location = 8) out vec3 fragBitangent;
#endif

layout(location = 9) out vec4 shadowCoord;

void main()
{
    mat4 modelView = frameUniforms.view * pcs.model;
	vec4 viewPos = modelView * vec4(inPosition, 1.0);
	gl_Position = frameUniforms.projection * viewPos;

    mat4 modelViewNormal = transpose(inverse(modelView));

#ifdef HAS_VERTEX_COLOR
    fragColor = inColor;
#endif
#ifdef HAS_TEX_COORD
    fragTexCoord = inTexCoord;
#endif
#ifdef HAS_NORMAL
    fragNormal = (modelViewNormal * vec4(inNormal, 0.0)).xyz;
#endif
#ifdef HAS_TANGENT
    fragTangent = (modelViewNormal * vec4(inTangent.xyz, 0.0)).xyz;
#endif

#ifdef HAS_BITANGENT
    vec3 inBitangent = cross(inNormal, inTangent.xyz) * inTangent.w;
    fragBitangent = (modelViewNormal * vec4(inBitangent, 0.0)).xyz;
#endif

    objectColor = objectUniforms.objectColor * materialUniforms.objectColor;

	lightVec = frameUniforms.lightPosition - viewPos.xyz;
	viewVec = viewPos.xyz;
    lightColor = frameUniforms.lightColor;

    shadowCoord = frameUniforms.lightViewProjection * pcs.model * vec4(inPosition, 1.0);
}
