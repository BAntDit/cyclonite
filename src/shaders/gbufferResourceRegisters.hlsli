
#ifndef GBUFFER_RESOURCE_REGISTERS_HLSLI
#define GBUFFER_RESOURCE_REGISTERS_HLSLI

#include "bindlessResourceRegisters.hlsli"

struct Vertex
{
    float3 position;
    float3 normal;
};

struct InstancedBatchData
{
    vk::BufferPointer<float4x4> transform;
    vk::BufferPointer<Vertex> geometry;
};

struct CBCommonGBufferPassData
{
    vk::BufferPointer<InstancedBatchData> batchData;
};

[[vk::binding(0, 2)]] ConstantBuffer<CBCommonGBufferPassData> commonGBufferPassData: register(b0, space2);

#endif // GBUFFER_RESOURCE_REGISTERS_HLSLI
