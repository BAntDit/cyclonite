#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vNormal;

layout (location = 0) out vec4 outNormal;
layout (location = 1) out vec4 outAlbedo;

void main() {
    outAlbedo = vec4(1., 0., 0., 1.);
    outNormal = vec4(vNormal, 1.);
}
