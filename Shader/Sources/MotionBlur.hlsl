SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);

Texture2D renderTarget : register(t0);
Texture2D velocity : register(t1);

#define SAMPLE_COUNT 10


struct MotionBlur_VIN
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct MotionBlur_VOUT
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD;
};

MotionBlur_VOUT MotionBlur_VS(MotionBlur_VIN input)
{
    MotionBlur_VOUT output;
    output.position = float4(input.position, 1.0f);
    output.texcoord = input.texcoord;
    return output;
}

float4 MotionBlur_PS(MotionBlur_VOUT input) : SV_Target
{
    
    
    float4 color = renderTarget.Sample(linearWrapSampler, input.texcoord);
    float4 velo = velocity.Sample(linearWrapSampler, input.texcoord);
    velo.xy /= -(float)SAMPLE_COUNT;
    
    int cnt = 1;
    float2 texCoord = input.texcoord;
    [unroll]
    for (int i = cnt; i < SAMPLE_COUNT; ++i)
    {
 
        float4 currentColor = renderTarget.Sample(linearWrapSampler, texCoord + velo.xy * (float) i);
        float4 currentVelo = velocity.Sample(linearWrapSampler, texCoord + velo.xy * (float) i);
        
        float len = length(currentColor.xyz);
        float maskZero = step(len, 0.0f);
        float4 defaultColor = renderTarget.Sample(linearWrapSampler, input.texcoord);
        currentColor = lerp(currentColor, defaultColor, maskZero);
        
        float mask = 1.0f - step(0.002f, abs(velo.a - currentVelo.a));
        [unroll]
        for (int i = 0; i < mask; ++i)
        {
            ++cnt;
            color += currentColor * mask;
        }

          
    } 
    
    
    return color / cnt;
}
