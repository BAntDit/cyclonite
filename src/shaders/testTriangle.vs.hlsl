
struct VertexData {
    float4 pos;
    float2 uv;
};

static const VertexData vData[3] = {
    {float4(0.5f, 0.5f, 0.5f, 1.0f), float2(1.f, 1.f)},
    {float4(-0.6f, 0.2f, 0.5f, 1.0f), float2(0.f, 1.f)},  // swapped
    {float4(-0.1f, -0.5f, 0.5f, 1.0f), float2(0.f, 0.f)}   // swapped
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOutput main(uint vertexId : SV_VertexID)
{
    VSOutput output;

    output.position = vData[vertexId].pos;
    output.uv = vData[vertexId].uv;

    return output;
}
