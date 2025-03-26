SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

Texture2D GBuffers[4] : register(t0, space0);

cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    float3 cameraPosition;
}

struct Deffered_VIN
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct Deffered_VOUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};


Deffered_VOUT Deffered_VS(Deffered_VIN input)
{
    Deffered_VOUT output;
    output.position = float4(input.position, 1.f);
    output.texcoord = input.texcoord;
    return output;
}


float ComputeShadowFactor(float3 worldPos)
{
    float4 lightPos = mul(float4(worldPos, 1.0), viewProjection);
    

    lightPos.xyz /= lightPos.w;

    lightPos.xy = lightPos.xy * 0.5f + 0.5f;

    if (lightPos.x < 0.0 || lightPos.x > 1.0 || lightPos.y < 0.0 || lightPos.y > 1.0)
        return 1.0;
    

    float shadowMapDepth = GBuffers[3].Sample(linearClampSampler, lightPos.xy).r;

    return (lightPos.z - 0.01 > shadowMapDepth) ? 0.0f : 1.0f;
}

float4 Deffered_PS(Deffered_VOUT input) : SV_TARGET
{

    // return float4(GBuffers[3].Sample(linearClampSampler, input.texcoord).rrr, 1.0);
    
    return float4(GBuffers[0].Sample(linearClampSampler, input.texcoord).rgb, 1.0);
}