#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

vec3 getBaseColor()
{
	// vec3 result = objectColor.rgb;

	// return result;

	return vec3(1.0, 0.0, 0.0);
}

void main()
{
	vec3 baseColor = getBaseColor();

	outColor = vec4(baseColor, 1.0);
}
