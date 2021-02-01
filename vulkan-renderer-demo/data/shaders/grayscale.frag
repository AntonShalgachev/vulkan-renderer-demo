#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0);
    float intensity = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    outColor = vec4(vec3(intensity), 1.0);
}