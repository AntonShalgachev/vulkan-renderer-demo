#version 450

layout(points) in;
layout(line_strip, max_vertices = 2) out;

layout(binding = 0) uniform UniformBufferObject {
    mat4 modelView;
    mat4 normal;
    mat4 projection;
    vec3 lightPosition;
    vec3 viewPos;
} ubo;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec3 outColor;

void main(void)
{	
	float normalLength = 0.1;
	
	vec3 pos = gl_in[0].gl_Position.xyz;
	vec3 normal = inNormal[0].xyz;

	vec4 viewPos = ubo.modelView * vec4(pos, 1.0);
	gl_Position = ubo.projection * viewPos;
    vec4 viewNormal = vec4(normalize(ubo.normal * vec4(normal, 0.0)).xyz, 1.0);

	outColor = abs(viewNormal.xyz);

	gl_Position = ubo.projection * viewPos;
	EmitVertex();

	gl_Position = ubo.projection * (viewPos + normalLength * viewNormal);
	EmitVertex();

	EndPrimitive();
}
