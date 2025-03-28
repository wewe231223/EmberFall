SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);


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


float ComputeShadowFactor(float4 shadowPosH, float bias)
{
    float depth = shadowPosH.z - bias;
    uint width, height, numMips;
    GBuffers[3].GetDimensions(0, width, height, numMips);
    float dx = 1.0f / (float) width;
    float dy = 1.0f / (float) height;

    const float2 offsets[9] =
    {
        float2(-dx, -dy), float2(0.0f, -dy), float2(dx, -dy),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, dy), float2(0.0f, dy), float2(dx, dy)
    };
    float percentLit = 0.0f;
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += GBuffers[3].SampleCmpLevelZero(PCFSampler, shadowPosH.xy + offsets[i], depth).r;
    }
    return percentLit / 9.0f;
}

float4 Deffered_PS(Deffered_VOUT input) : SV_TARGET
{


    //return float4(GBuffers[3].Sample(linearWrapSampler, input.texcoord).rrr, 1.0f);

    float4 diffuse = GBuffers[0].Sample(linearWrapSampler, input.texcoord);
    float4 worldPos = GBuffers[2].Sample(linearWrapSampler, input.texcoord);
    if (worldPos.x == 0.0f)
    {
        return diffuse;
    }
    float4 texPos = mul(worldPos, viewProjection);
    texPos.x = texPos.x * 0.5f + 0.5f;
    texPos.y = -texPos.y * 0.5f + 0.5f;

    if (texPos.x < 0.0 || texPos.x > 1.0 || texPos.y < 0.0 || texPos.y > 1.0)
    {
        return diffuse;
    }


    float bias = 0.006f;

    float shadowFactor = 0.5f;
    //float shadowFactor = GBuffers[3].SampleCmpLevelZero(PCFSampler, texPos.xy, texPos.z); //2x2 PCF
    //float shadowFactor = ComputeShadowFactor(texPos, bias); //3x3 PCF



    float depth = GBuffers[3].Sample(linearWrapSampler, texPos.xy);


    if (texPos.z > (depth + bias))
    {

        return diffuse * shadowFactor;
    }
    return diffuse;


    // return float4(GBuffers[3].Sample(linearClampSampler, input.texcoord).rrr, 1.0);

    //return float4(GBuffers[0].Sample(linearClampSampler, input.texcoord).rgb, 1.0);

}