#version 450
#extension GL_ARB_separate_shader_objects : enable

// #define HAS_VERTEX_COLOR 0
// #define HAS_TEXTURE 0
// #define HAS_NORMAL 0
// #define HAS_TEXTURE 0

#ifdef HAS_TEXTURE
layout(binding = 1) uniform sampler2D texSampler;
#endif

#ifdef HAS_NORMAL
#define HAS_LIGHT
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

layout(location = 3) in vec3 viewVec;
layout(location = 4) in vec3 lightVec;

layout(location = 5) in vec4 objectColor;

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

void main()
{
	vec3 baseColor = getBaseColor();

#ifdef HAS_LIGHT
	vec3 ambient = baseColor * vec3(0.25);
	vec3 N = normalize(fragNormal);
	vec3 L = normalize(lightVec);
	vec3 V = normalize(viewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), 0.0) * baseColor;
	float specular = pow(max(dot(R, -V), 0.0), 32.0);

	outColor = vec4(0.0);
	outColor += vec4(ambient, 1.0);
	outColor += vec4(diffuse, 1.0);
	outColor += vec4(vec3(specular), 1.0);
#else
	outColor = vec4(baseColor, 1.0);
#endif
}
