#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform sampler2D samplerNormal;
layout (set = 0, binding = 1) uniform sampler2D samplerAlbedo;

layout(location = 0) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 norm = texture(samplerNormal, vUV).rgb;
    vec3 albedo = texture(samplerAlbedo, vUV).rgb;

    vec3 outC = abs(norm) * vec3(0.2126, 0.7152, 0.0722) * 0.33;
    outC *= albedo.r;

    outColor = vec4(outC, 1.0);
}
