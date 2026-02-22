
struct Vertex
{
    float3 position;
    float3 normal;
};

struct Instance
{
    float4 transform1;
    float4 transform2;
    float4 transform3;
};

struct CameraBuffer
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjMatrix;
};

StructuredBuffer<Vertex> vertices : register(t0);
StructuredBuffer<Instance> instances : register(t1);
ConstantBuffer<CameraBuffer> camera : register(b0);

struct VSOutput
{
    float4 position : SV_Position;
    float3 normal : TEXCOORD0;
};

VSOutput main(
    uint vertexId : SV_VertexID,
    uint instanceId : SV_InstanceID
)
{
    VSOutput output;

    float4x4 matrixWorld = float4x4(
        instances[instanceId].transform1,
        instances[instanceId].transform2,
        instances[instanceId].transform3,
        float4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    matrixWorld = transpose(matrixWorld);

    Vertex vertex = vertices[vertexId];

    float3 position = vertex.position;
    float3 normal = vertex.normal;

    float4 worldPosition = mul(float4(position, 1.0f), matrixWorld);

    output.position = mul(worldPosition, camera.viewProjMatrix);
    output.normal = normal;

    return output;
}

