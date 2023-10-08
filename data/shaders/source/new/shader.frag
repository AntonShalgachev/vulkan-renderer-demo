#version 450
#extension GL_ARB_separate_shader_objects : enable

// layout(set = 0, binding = 3) uniform sampler2D shadowMap; // TODO enable

#ifdef HAS_TEXTURE
layout(set = 1, binding = 1) uniform sampler2D texSampler;
#endif

#ifdef HAS_NORMAL_MAP
layout(set = 1, binding = 2) uniform sampler2D normalMapSampler;
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
layout(location = 6) in vec3 lightColor;

layout(location = 7) in vec4 objectColor;

#ifdef HAS_BITANGENT
layout(location = 8) in vec3 fragBitangent;
#endif

layout(location = 9) in vec4 shadowCoord;

layout(location = 0) out vec4 outColor;

float gamma = 2.2;

vec3 getBaseColor()
{
	vec3 result = objectColor.rgb;

#if defined(HAS_TEX_COORD) && defined(HAS_TEXTURE)
	result *= pow(texture(texSampler, fragTexCoord).rgb, vec3(gamma));
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

float computeShadowFactor(vec4 shadowCoord)
{
	// if (abs(shadowCoord.x) > 1.0)
	// 	return 0.1;
	// if (abs(shadowCoord.y) > 1.0)
	// 	return 0.1;
	// if (abs(shadowCoord.z) > 1.0)
	// 	return 0.1;

	// vec2 uv = shadowCoord.xy * 0.5 + 0.5;
	// uv.y *= -1.0;

	// float dist = texture(shadowMap, uv).r;
	// if (shadowCoord.z > 0.0 && dist < shadowCoord.z)
	// 	return 0.1;

	return 1.0;
}

const float PI = 3.14159265359;
const float metallic = 0.0;
const vec3 baseDielectricReflectivity = vec3(0.04);
const float roughness = 0.5;
const float ao = 1;

#ifdef HAS_LIGHT
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 applyLighting(vec3 albedo)
{
	vec3 N = getNormalVector();
	vec3 V = normalize(viewVec);

	vec3 baseMetallicReflectivity = albedo;
	vec3 F0 = mix(baseDielectricReflectivity, baseMetallicReflectivity, metallic);

	vec3 lightVecs[1] = vec3[](lightVec);
	vec3 lightColors[1] = vec3[](lightColor);

	vec3 L0 = vec3(0.0);
	for(int i = 0; i < 1; i++)
	{
		// calculate per-light radiance
		vec3 L = normalize(lightVecs[i]);
		vec3 H = normalize(V + L);
		float distance = length(lightVecs[i]);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;
		
		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;
			
		// add to outgoing radiance L0
		float NdotL = max(dot(N, L), 0.0);
		L0 += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + L0;

	return color;
}
#endif

void main()
{
	vec3 color = getBaseColor();

#ifdef HAS_LIGHT
	color = applyLighting(color);
#endif

	color *= computeShadowFactor(shadowCoord / shadowCoord.w);

	color = color / (color + vec3(1.0));

	outColor = vec4(color, 1.0);
}
