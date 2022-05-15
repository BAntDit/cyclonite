#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform sampler2D samplerNormal;
layout (set = 0, binding = 1) uniform sampler2D samplerAlbedo;

layout(location = 0) in vec2 vUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv1 = vec2(vUV.x * 2., vUV.y);
    vec3 norm = texture(samplerNormal, uv1).rgb;

    vec2 uv2 = vec2(vUV.x * 2. - 1., vUV.y);
    vec3 albedo = texture(samplerAlbedo, uv2).rgb;
    vec3 norm2 = texture(samplerNormal, uv2).rgb;

    albedo = vec3(dot(albedo, abs(norm2))) * 0.33;

    norm = norm * step(vUV.x, 0.5);
    albedo = albedo * step(0.5, vUV.x);

    vec3 outC = abs(norm) + albedo;

    outColor = vec4(outC, 1.0);
}
