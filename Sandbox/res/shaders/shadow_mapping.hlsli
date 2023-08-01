
float LinearizeDepth(float z, float n, float f)
{
    return -f * n / (f * z - n * z - f);
}

float ShadowMappingPCF3x3(Texture2D<float> smap, SamplerComparisonState smapSampler, float4 pixelLightSpace)
{
    float3 projCoord = pixelLightSpace.xyz / pixelLightSpace.w;
    
    if (projCoord.z > 1.0f)
        return 1.0f;
    
    projCoord.x = projCoord.x * 0.5f + 0.5f;
    projCoord.y = -projCoord.y * 0.5f + 0.5f;
    
    float sum = 0.0f;
    const int range = 1;
    [unroll]
    for (int x = -range; x <= range; x++)
    {
        [unroll]
        for (int y = -range; y <= range; y++)
        {
            sum += smap.SampleCmpLevelZero(smapSampler, projCoord.xy, projCoord.z, int2(x, y));
        }
    }

    return sum / ((range * 2.0f + 1.0f) * (range * 2.0f + 1.0f));
}

float ShadowMappingPCF5x5(Texture2D<float> smap, SamplerComparisonState smapSampler, float4 pixelLightSpace)
{
    float3 projCoord = pixelLightSpace.xyz / pixelLightSpace.w;
    
    if (projCoord.z > 1.0f)
        return 1.0f;
    
    projCoord.x = projCoord.x * 0.5f + 0.5f;
    projCoord.y = -projCoord.y * 0.5f + 0.5f;
    
    float sum = 0.0f;
    const int range = 2;
    [unroll]
    for (int x = -range; x <= range; x++)
    {
        [unroll]
        for (int y = -range; y <= range; y++)
        {
            sum += smap.SampleCmpLevelZero(smapSampler, projCoord.xy, projCoord.z, int2(x, y));
        }
    }

    return sum / ((range * 2.0f + 1.0f) * (range * 2.0f + 1.0f));
}

float ShadowMappingPCF7x7(Texture2D<float> smap, SamplerComparisonState smapSampler, float4 pixelLightSpace)
{
    float3 projCoord = pixelLightSpace.xyz / pixelLightSpace.w;
    
    if (projCoord.z > 1.0f)
        return 1.0f;
    
    projCoord.x = projCoord.x * 0.5f + 0.5f;
    projCoord.y = -projCoord.y * 0.5f + 0.5f;
    
    float sum = 0.0f;
    const int range = 3;
    [unroll]
    for (int x = -range; x <= range; x++)
    {
        [unroll]
        for (int y = -range; y <= range; y++)
        {
            sum += smap.SampleCmpLevelZero(smapSampler, projCoord.xy, projCoord.z, int2(x, y));
        }
    }

    return sum / ((range * 2.0f + 1.0f) * (range * 2.0f + 1.0f));
}

float ShadowMappingPCF9x9(Texture2D<float> smap, SamplerComparisonState smapSampler, float4 pixelLightSpace)
{
    float3 projCoord = pixelLightSpace.xyz / pixelLightSpace.w;
    
    if (projCoord.z > 1.0f)
        return 1.0f;
    
    projCoord.x = projCoord.x * 0.5f + 0.5f;
    projCoord.y = -projCoord.y * 0.5f + 0.5f;
    
    float sum = 0.0f;
    const int range = 4;
    [unroll]
    for (int x = -range; x <= range; x++)
    {
        [unroll]
        for (int y = -range; y <= range; y++)
        {
            sum += smap.SampleCmpLevelZero(smapSampler, projCoord.xy, projCoord.z, int2(x, y));
        }
    }

    return sum / ((range * 2.0f + 1.0f) * (range * 2.0f + 1.0f));
}

// pcfRange 
//  0 = no pcf
//  1 = 3x3
//  2 = 5x5
//  3 = 7x7
//  4 = 9x9
float ShadowMapping(Texture2D<float> smap, SamplerComparisonState smapSampler, float4 pixelLightSpace, uint pcfRange)
{
    float shadow = 1.0f;
    
    switch (pcfRange)
    {
    case 0:
        float3 projCoord = pixelLightSpace.xyz / pixelLightSpace.w;
        if (projCoord.z > 1.0f)
            return shadow;
    
        projCoord.x = projCoord.x * 0.5f + 0.5f;
        projCoord.y = -projCoord.y * 0.5f + 0.5f;
        shadow = smap.SampleCmpLevelZero(smapSampler, projCoord.xy, projCoord.z);
        break;
    
    case 1:
        shadow = ShadowMappingPCF3x3(smap, smapSampler, pixelLightSpace);
        break;
    
    case 2:
        shadow = ShadowMappingPCF5x5(smap, smapSampler, pixelLightSpace);
        break;
    
    case 3:
        shadow = ShadowMappingPCF7x7(smap, smapSampler, pixelLightSpace);
        break;
    
    case 4:
        shadow = ShadowMappingPCF9x9(smap, smapSampler, pixelLightSpace);
        break;
    }
    
    return shadow;
}

float VarianceShadowMapping(Texture2D<float2> smap, SamplerState smapSampler, float4 pixelLightSpace, float minVariance, float lightBleedReduction)
{
    float3 projCoord = pixelLightSpace.xyz / pixelLightSpace.w;
    
    if (projCoord.z > 1.0f)
        return 1.0f;
    
    //projCoord.z = LinearizeDepth(projCoord.z, 0.1f, 500.0f);
    
    projCoord.x = projCoord.x * 0.5f + 0.5f;
    projCoord.y = -projCoord.y * 0.5f + 0.5f;
    
    // http://igm.univ-mlv.fr/~biri/Enseignement/MII2/Donnees/variance_shadow_maps.pdf
    // https://developer.download.nvidia.com/SDK/10/direct3d/Source/VarianceShadowMapping/Doc/VarianceShadowMapping.pdf
    // Chevychev inequality
    // (M_1, M_2) = (mean, mean^2) = (depth, depth^2)
    // variance = M_2 - M_1^2
    // pMax(t) = variance / (variance + (t - M_1)^2)
    
    
    float2 moments = smap.Sample(smapSampler, projCoord.xy);
    float variance = max(moments.y - (moments.x * moments.x), minVariance);
    float meanDist = moments.x - projCoord.z;
    float pMax = variance / (variance + meanDist * meanDist);
    
    return max(smoothstep(lightBleedReduction, 1.0f, pMax), projCoord.z <= moments.x);
}