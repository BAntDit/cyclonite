#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Vertex
{
    vec3 position;
    vec3 normal;
};

struct Instance
{
    vec4 transform1;
    vec4 transform2;
    vec4 transform3;
};

layout(std430, set = 0, binding = 0) buffer vertices {
    Vertex vertex[];
};

layout(std430, set = 0, binding = 1) buffer instances {
    Instance instance[];
};

layout(std140, set = 0, binding = 2) uniform camera {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjMatrix;
};

layout(location = 0) out vec3 vNormal;

void main() {
    mat4 matrixWorld = transpose(mat4(
            instance[gl_InstanceIndex].transform1,
            instance[gl_InstanceIndex].transform2,
            instance[gl_InstanceIndex].transform3,
            vec4(0., 0., 0., 1.))
        );

    vec3 position = vertex[gl_VertexIndex].position;
    vec3 normal = vertex[gl_VertexIndex].normal;

    vec4 worldPosition = matrixWorld * vec4(position, 1.0);

    gl_Position = viewProjMatrix * worldPosition;

    vNormal = normal; // TODO:: compute view sapce normal
}
