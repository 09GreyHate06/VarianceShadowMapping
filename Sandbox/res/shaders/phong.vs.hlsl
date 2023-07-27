struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 normal : NORMAL;
    float3 pixelWorldSpacePos : PIXEL_WORLD_SPACE_POS;
    float3 viewPos : VIEW_POS;
};

cbuffer SystemCBuf : register(b0)
{
    float4x4 viewProjection;
    float3 viewPos; // camera pos
    float p0;
};

cbuffer EntityCBuf : register(b1)
{
    float4x4 transform;
    float4x4 normalMatrix;
};

VSOutput main(VSInput input)
{
    float4 pixelWorldSpacePos = mul(float4(input.position, 1.0f), transform);
    
    VSOutput vso;
    vso.position = mul(pixelWorldSpacePos, viewProjection);
    vso.uv = input.uv;
    vso.tangent = mul(input.tangent, (float3x3)normalMatrix);
    vso.bitangent = mul(input.bitangent, (float3x3)normalMatrix);
    vso.normal = mul(input.normal, (float3x3)normalMatrix);
    vso.pixelWorldSpacePos = pixelWorldSpacePos.xyz;
    vso.viewPos = viewPos;
    
    return vso;
}