#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Transform
{
    vec4 xAxisTx;
    vec4 yAxisTy;
    vec4 zAxisTz;
};

layout(std430, set = 0, binding = 0) readonly buffer xfrms
{
    Transform transforms[];
};

layout(std140, set = 0, binding = 1) uniform camera {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjMatrix;
};

layout(location = 0) out vec3 vNormal;

vec3 positions[24] = vec3[](
    vec3(-1.0, -1.0, 1.0),
    vec3(1.0, -1.0, 1.0),
    vec3(1.0, 1.0, 1.0),
    vec3(-1.0, 1.0, 1.0),

    vec3(1.0, 1.0, 1.0),
    vec3(1.0, 1.0, -1.0),
    vec3(1.0, -1.0, -1.0),
    vec3(1.0, -1.0, 1.0),

    vec3(-1.0, -1.0, -1.0),
    vec3(1.0, -1.0, -1.0),
    vec3(1.0, 1.0, -1.0),
    vec3(-1.0, 1.0, -1.0),

    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, 1.0),
    vec3(-1.0, 1.0, 1.0),
    vec3(-1.0, 1.0, -1.0),

    vec3(1.0, 1.0, 1.0),
    vec3(-1.0, 1.0, 1.0),
    vec3(-1.0, 1.0, -1.0),
    vec3(1.0, 1.0, -1.0),

    vec3(-1.0, -1.0, -1.0),
    vec3(1.0, -1.0, -1.0),
    vec3(1.0, -1.0, 1.0),
    vec3(-1.0, -1.0, 1.0)
);

vec3 normals[24] = vec3[](
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0),

    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),

    vec3(0.0, 0.0, -1.0),
    vec3(0.0, 0.0, -1.0),
    vec3(0.0, 0.0, -1.0),
    vec3(0.0, 0.0, -1.0),

    vec3(-1.0, 0.0, 0.0),
    vec3(-1.0, 0.0, 0.0),
    vec3(-1.0, 0.0, 0.0),
    vec3(-1.0, 0.0, 0.0),

    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0),

    vec3(0.0, -1.0, 0.0),
    vec3(0.0, -1.0, 0.0),
    vec3(0.0, -1.0, 0.0),
    vec3(0.0, -1.0, 0.0)
);

void main() {
    mat4 worldMatrix = mat4(
        transforms[gl_InstanceIndex].xAxisTx,
        transforms[gl_InstanceIndex].yAxisTy,
        transforms[gl_InstanceIndex].zAxisTz,
        vec4(0.0, 0.0, 0.0, 1.0));

    vec3 position = positions[gl_VertexIndex];

    vec4 worldPosition = vec4(position, 1.0) * worldMatrix;

    gl_Position = viewProjMatrix * worldPosition;

    vNormal = normals[gl_VertexIndex];
}
