
float2 main(float4 position : SV_Position) : SV_Target
{
    //https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps
    //slope scaled bias. We already have hardware slope scaled bias
    //float depth = position.z;
    //float dx = ddx(depth);
    //float dy = ddy(depth);
    //float moment2 = depth * depth + 0.25 * (dx * dx + dy * dy);
    //return float2(depth, moment2);
    
    
    return float2(position.z, position.z * position.z);
}