
cbuffer SystemCBuf : register(b0)
{
    float4x4 viewProj;
};

cbuffer EntityCBuf : register(b1)
{
    float4x4 transform;
};

float4 main(float3 position : POSITION) : SV_Position
{
    return mul(float4(position, 1.0f), mul(transform, viewProj));
}