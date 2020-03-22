#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, set = 0, binding = 0) uniform camera {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjMatrix;
};

layout(location = 0) out vec3 vColor;

vec3 positions[3] = vec3[](
    vec3(-0.5, -0.5, 0.5),
    vec3(0.5, -0.5, 0.5),
    vec3(0.5, 0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    vec3 position = positions[gl_VertexIndex];

    vec4 worldPosition = vec4(position, 1.0);

    gl_Position = viewProjMatrix * worldPosition;

    vColor = colors[gl_VertexIndex];
}
