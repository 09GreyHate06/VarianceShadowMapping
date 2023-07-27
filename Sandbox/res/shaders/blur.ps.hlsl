
cbuffer KernelCBuf : register(b0)
{
    uint nTaps;
    uint3 p0;
    float4 coefficients[35];
};

cbuffer ControlCBuf : register(b1)
{
    float2 texelStep;
    float2 p1;
};

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    const int radius = nTaps / 2;
    float4 maxColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    for (int i = -radius; i <= radius; i++)
    {
        const float2 tc = uv + float2(texelStep.x * i, texelStep.y * i);
        const float4 s = tex.Sample(samplerState, tc);
        const float coef = coefficients[i + radius];
        maxColor += s * coef;
    }

    return maxColor;
}