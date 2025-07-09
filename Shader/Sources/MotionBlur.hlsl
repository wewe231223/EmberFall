SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);

Texture2D renderTarget : register(t0);
Texture2D velocity : register(t1);

#define SAMPLE_COUNT 11

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
    output.position = float4(input.position, 1.f);
    output.texcoord = input.texcoord;
    return output;
}

float4 MotionBlur_PS(MotionBlur_VOUT input) : SV_Target
{
    
    
    float4 color = renderTarget.Sample(linearClampSampler, input.texcoord);
    float4 velo = velocity.Sample(linearClampSampler, input.texcoord);
    velo.xy /= -10.0f;
    int cnt = 1;
    float2 texCoord = input.texcoord;
    for (int i = cnt; i < SAMPLE_COUNT; ++i)
    {
 
        float4 currentColor = renderTarget.Sample(linearClampSampler, texCoord + velo.xy * (float)i);
        float4 currentVelo = velocity.Sample(linearClampSampler, texCoord + velo.xy * (float) i);
        if (length(currentColor.xyz) == 0)
        {
            currentColor = renderTarget.Sample(linearClampSampler, input.texcoord);

        }
        if (abs(velo.a - currentVelo.a) < 0.006f)
        {
            ++cnt;
            color += currentColor;
        }
    } 
    float4 finalColor = color / cnt;
    
    return finalColor;
}
