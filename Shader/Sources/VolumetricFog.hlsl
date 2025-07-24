SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);


cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
    float4x4 middleViewProjection;
    float4x4 prevViewProjection;
    float4x4 invView;
    float4x4 invProjection;

    float3 cameraPosition;
    int isShadow;
    float3 shadowOffset;
};

Texture3D fogVolume : register(t0);
Texture2D velocity : register(t1);

static float fogBegin = 0.0f;
static float fogEnd = 600.0f;

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION0;
    float3 viewRay : POSITION1;
    float2 texcoord : TEXCOORD;
};

float3 ApplyToneMap(float3 color)
{
    
    color *= 0.4f;
    float3 low = pow(abs(color * 0.38317f), 0.455f);
    float3 high = 1.0f - exp(-color);

    float3 mask = step(color, 1.413f);
    

    return lerp(high, low, mask);
}


VS_OUTPUT VolumetricFog_VS(VS_INPUT input)
{
    VS_OUTPUT output;

    output.texcoord = input.texcoord;
    output.position = float4(input.position, 1.0f);


    

    float4 viewRay = mul(float4(output.position.xy, 1.0f, 1.0f), invProjection);
    viewRay /= viewRay.w;
    output.viewRay = viewRay.xyz;


    return output;
}

float4 VolumetricFog_PS(VS_OUTPUT input) : SV_Target
{
    float viewSpaceLength = velocity.Sample(linearClampSampler, input.texcoord).z;
    
    viewSpaceLength = clamp(viewSpaceLength, fogBegin, fogEnd);


    float3 viewPosition = normalize(input.viewRay) * viewSpaceLength;

    
    
    
    float ndcZ = pow(saturate((length(viewPosition) - fogBegin) / (fogEnd - fogBegin)),  0.5f);

    
    float3 uv = float3(input.texcoord, 1.0 - ndcZ);
    float4 scatteringFog = fogVolume.Sample(linearClampSampler, uv);
    float3 scatteringColor = ApplyToneMap(scatteringFog.rgb);


    
    
    return float4(scatteringColor, scatteringFog.a);

}