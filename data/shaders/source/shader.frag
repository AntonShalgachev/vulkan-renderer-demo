#version 450
#extension GL_ARB_separate_shader_objects : enable

#ifdef HAS_TEXTURE
layout(binding = 1) uniform sampler2D texSampler;
#endif

#ifdef HAS_NORMAL_MAP
layout(binding = 2) uniform sampler2D normalMapSampler;
#endif

#ifdef HAS_NORMAL
#define HAS_LIGHT
#endif

#if defined(HAS_NORMAL) && defined(HAS_TANGENT)
#define HAS_BITANGENT
#endif

#ifdef HAS_VERTEX_COLOR
layout(location = 0) in vec3 fragColor;
#endif

#ifdef HAS_TEX_COORD
layout(location = 1) in vec2 fragTexCoord;
#endif

#ifdef HAS_NORMAL
layout(location = 2) in vec3 fragNormal;
#endif

#ifdef HAS_TANGENT
layout(location = 3) in vec3 fragTangent;
#endif

layout(location = 4) in vec3 viewVec;
layout(location = 5) in vec3 lightVec;

layout(location = 6) in vec4 objectColor;

#ifdef HAS_BITANGENT
layout(location = 7) in vec3 fragBitangent;
#endif

layout(location = 0) out vec4 outColor;

vec3 getBaseColor()
{
	vec3 result = objectColor.rgb;

#if defined(HAS_TEX_COORD) && defined(HAS_TEXTURE)
	result *= texture(texSampler, fragTexCoord).rgb;
#endif

#ifdef HAS_VERTEX_COLOR
	result *= fragColor;
#endif

return result;
}

#ifdef HAS_NORMAL
vec3 getNormalVector()
{
#if defined(HAS_TANGENT) && defined(HAS_BITANGENT) && defined(HAS_NORMAL_MAP) && defined(HAS_TEX_COORD)
	vec3 N = normalize(fragNormal);
	vec3 T = normalize(fragTangent);
	vec3 B = normalize(fragBitangent);

	mat3 TBN = mat3(T, B, N);

	vec3 normalInTangentSpace = texture(normalMapSampler, fragTexCoord).rgb * 2.0 - 1.0;
	
	return normalize(TBN * normalInTangentSpace);
#else
	return normalize(fragNormal);
#endif
}
#endif

void main()
{
	vec3 baseColor = getBaseColor();

#ifdef HAS_LIGHT
	vec3 ambient = baseColor * vec3(0.25);
	vec3 N = getNormalVector();
	vec3 L = normalize(lightVec);
	vec3 V = normalize(viewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = abs(dot(N, L)) * baseColor;
	float specular = pow(max(dot(R, -V), 0.0), 32.0);

	outColor = vec4(0.0);
	outColor += vec4(ambient, 1.0);
	outColor += vec4(diffuse, 1.0);
	outColor += vec4(vec3(specular), 1.0);
#else
	outColor = vec4(baseColor, 1.0);
#endif
}
