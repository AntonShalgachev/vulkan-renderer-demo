#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 2) in vec3 fragNormal;

layout(location = 3) in vec3 viewVec;
layout(location = 4) in vec3 lightVec;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 color = vec4(1.0, 0.0, 1.0, 1.0);

	vec3 ambient = color.rgb * vec3(0.25);
	vec3 N = normalize(fragNormal);
	vec3 L = normalize(lightVec);
	vec3 V = normalize(viewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), 0.0) * color.rgb;
	float specular = pow(max(dot(R, -V), 0.0), 32.0);

	outColor = vec4(0.0);
	outColor += vec4(ambient, 1.0);
	outColor += vec4(diffuse, 1.0);
	outColor += vec4(vec3(specular), 1.0);
}
