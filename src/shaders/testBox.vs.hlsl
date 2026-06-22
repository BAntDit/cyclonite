

struct VertexData {
    float3 position;
    float3 normal;
    float2 uv0;
};

static const VertexData vDataNonIndexed[] = {
    // Front face (z = +0.5)
    // Triangle 1
    { {-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
    { { 0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
    { { 0.5f,  0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
    // Triangle 2
    { { 0.5f,  0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
    { {-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },

    // Back face (z = -0.5)
    // Triangle 1
    { { 0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },
    { {-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
    // Triangle 2
    { {-0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },

    // Left face (x = -0.5)
    // Triangle 1
    { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
    { {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
    // Triangle 2
    { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },

    // Right face (x = +0.5)
    // Triangle 1
    { { 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
    { { 0.5f, -0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
    { { 0.5f,  0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
    // Triangle 2
    { { 0.5f,  0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },

    // Top face (y = +0.5)
    // Triangle 1
    { {-0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
    { { 0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
    { { 0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
    // Triangle 2
    { { 0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
    { {-0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },

    // Bottom face (y = -0.5)
    // Triangle 1
    { {-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
    { { 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
    { { 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
    // Triangle 2
    { { 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
    { {-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
};

struct CameraBuffer
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjMatrix;
};


[[vk::binding(0, 2)]] ConstantBuffer<CameraBuffer> camera: register(b0, space2);

struct VSOutput
{
    float4 position : SV_Position;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

VSOutput main(uint vertexId : SV_VertexID)
{
    VSOutput output;

    Vertex vertex = vertices[vertexId];

    float3 position = vertex.position;
    float3 normal = vertex.normal;

    float4 worldPosition = position;

    output.position = mul(worldPosition, camera.viewProjMatrix);
    output.normal = normal;
    output.uv = uv;

    return output;
}
