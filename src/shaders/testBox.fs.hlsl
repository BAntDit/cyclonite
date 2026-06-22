

struct PSInput
{
    float4 position : SV_Position;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

struct PSOutput
{
    float4 color : SV_Target0;
};


PSOutput main(PSInput input)
{
    PSOutput output;

    output.color = float4(input.uv.x, input.uv.y, 0.0f, 1.0f);

    return output;
}

































