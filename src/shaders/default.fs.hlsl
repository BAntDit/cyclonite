
struct PSInput
{
    float4 position : SV_Position;
    float3 normal : TEXCOORD0;
};

struct PSOutput
{
    float4 normal : SV_Target0;
    float4 albedo : SV_Target1;
};

PSOutput main(PSInput input)
{
PSOutput output;

output.albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
output.normal = float4(input.normal, 1.0f);

return output;
}
