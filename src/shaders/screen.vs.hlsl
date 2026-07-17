
void main(
    uint vertexId : SV_VertexID,
    out float4 position : SV_Position,
    out float2 uv : TEXCOORD0
)
{
    uv = float2((vertexId << 1) & 2, vertexId & 2);
    position = float4(uv * 2.0f - 1.0f, 0.0f, 1.0f);
}
