#include "bump_mapping.hlsli"
#include "phong.hlsli"
#include "shadow_mapping.hlsli"

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
    struct DirectionalLight
    {
        float3 color;
        float ambientIntensity;
        float3 direction;
        float intensity;
        
        float4x4 lightSpace;
    } dirLight;
    
    struct VSMControl
    {
        bool enabled;
        float minVariance;
        float lightBleedReduction;
        float p0;
    } vsmControl;
    
    struct BasicSMapControl
    {
        uint pcfLevel;
        float3 p0;
    } basicSMapControl;
};

cbuffer EntityCBuf : register(b1)
{
    struct Material
    {
        float4 color;
        float2 tiling;
        float shininess;
        bool enableNormalMapping;
        bool enableParallaxMapping;
        float depthMapScale;
        int p0;
        int p1;
    } mat;
};

Texture2D<float4> diffuseMap : register(t0);
Texture2D<float3> normalMap : register(t1);
Texture2D<float> depthMap : register(t2);
SamplerState matSampler : register(s0);

Texture2D<float> smap : register(t3);
SamplerComparisonState smapSampler : register(s1);

Texture2D<float2> vsm : register(t4);
SamplerState vsmSampler : register(s2);

float4 main(VSOutput input) : SV_Target
{
    float3 tangent = normalize(input.tangent);
    float3 bitangent = normalize(input.bitangent);
    float3 normal = normalize(input.normal);
    float3 pixelToView = normalize(input.viewPos - input.pixelWorldSpacePos);
    float2 uv = input.uv * mat.tiling;

    if (mat.enableNormalMapping)
    {
        float3x3 tbn = TBNOrthogonalized(tangent, normal);
        if (mat.enableParallaxMapping)
        {
            uv = ParallaxOcclusionMapping(depthMap, matSampler, mat.depthMapScale, uv, mul(pixelToView, transpose(tbn)));
            // cant work all the time because some loaded 3d model have negative or greater than 1 UV's
            if (uv.x > mat.tiling.x || uv.y > mat.tiling.y || uv.x < 0.0 || uv.y < 0.0) 
                clip(-1);
        }
        
        normal = NormalMapping(normalMap.Sample(matSampler, uv), tbn);
    }
    
    float4 pixelCol = diffuseMap.Sample(matSampler, uv) * mat.color;
    
    clip(pixelCol.a - 0.0001f);
    
    // light calculation
    float shadow = 1.0f;
    if (vsmControl.enabled)
        shadow = VarianceShadowMapping(vsm, vsmSampler, mul(float4(input.pixelWorldSpacePos, 1.0f), dirLight.lightSpace), vsmControl.minVariance, vsmControl.lightBleedReduction);
    else
        shadow = ShadowMapping(smap, smapSampler, mul(float4(input.pixelWorldSpacePos, 1.0f), dirLight.lightSpace), basicSMapControl.pcfLevel);
    
    float3 pixelToLight = normalize(-dirLight.direction);
    float3 phong = Phong(dirLight.color, pixelToLight, pixelToView, normal, dirLight.ambientIntensity, dirLight.intensity, mat.shininess, shadow);
    
    return float4(phong * pixelCol.rgb, pixelCol.a);
}