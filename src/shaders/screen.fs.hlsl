Texture2D<float4> samplerNormal : register(t0);
Texture2D<float4> samplerAlbedo : register(t1);
SamplerState g_sampler : register(s0); // Default sampler

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

PSOutput main(PSInput input)
{
    PSOutput output;

    // Calculate UV coordinates for left/right split screen
    float2 uv1 = float2(input.uv.x * 2.0f, input.uv.y);
    float3 norm = samplerNormal.Sample(g_sampler, uv1).rgb;

    float2 uv2 = float2(input.uv.x * 2.0f - 1.0f, input.uv.y);
    float3 albedo = samplerAlbedo.Sample(g_sampler, uv2).rgb;
    float3 norm2 = samplerNormal.Sample(g_sampler, uv2).rgb;

    // Calculate albedo based on dot product with absolute normal
    albedo = float3(dot(albedo, abs(norm2))) * 0.33f;

    // Mask based on horizontal position (split screen effects)
    norm = norm * step(input.uv.x, 0.498f);
    albedo = albedo * step(0.502f, input.uv.x);

    // White strip in the middle
    float3 white = float3(1.0f, 1.0f, 0.0f) *
    step(0.498f, input.uv.x) *
    step(input.uv.x, 0.502f);

    // Combine results
    float3 outC = abs(norm) + albedo + white;

    output.color = float4(outC, 1.0f);

    return output;
}