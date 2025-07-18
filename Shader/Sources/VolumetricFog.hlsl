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

static float fogBegin = 100.0f;
static float fogEnd = 500.0f;

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
    float3 worldRay : POSITION2;
    float2 texcoord : TEXCOORD;
};

float3 HDR(float3 l)
{
    //l = l * VolumetricFogParam.Exposure;
    l.r = l.r < 1.413f ? pow(abs(l.r * 0.38317f), 1.f / 2.2f) : 1.f - exp(-l.r);
    l.g = l.g < 1.413f ? pow(abs(l.g * 0.38317f), 1.f / 2.2f) : 1.f - exp(-l.g);
    l.b = l.b < 1.413f ? pow(abs(l.b * 0.38317f), 1.f / 2.2f) : 1.f - exp(-l.b);
    return l;
}


VS_OUTPUT VolumetricFog_VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.texcoord = input.texcoord;
    output.position = float4(input.position, 1.0f);


    float4 worldPosition = mul(mul(float4(output.position.xy, 0.0f, 1.0f), invProjection), invView);
    output.worldPosition = worldPosition.xyz / worldPosition.w;

    float4 viewRay = mul(float4(output.position.xy, 1.0f, 1.0f), invProjection);
    viewRay /= viewRay.w;
    output.viewRay = viewRay.xyz;

    float4 worldRay = mul(mul(float4(output.position.xy, 1.0f, 1.0f), invProjection), invView);
    worldRay /= worldRay.w;
    output.worldRay = worldRay.xyz;

    return output;
}

float4 VolumetricFog_PS(VS_OUTPUT input) : SV_Target
{
    float viewSpaceDistance = velocity.Sample(linearClampSampler, input.texcoord).z;
    if (viewSpaceDistance <= 0.f)
    {
        viewSpaceDistance = fogEnd;
    }

    float3 viewPosition = normalize(input.viewRay) * viewSpaceDistance;

    
    
    
    
    //float3 uv = float3(input.texcoord, pow(viewPosition.z, 2) * (fogEnd - fogBegin) + fogBegin);
    float linearDepth = pow(viewPosition.z, 2) * (fogEnd - fogBegin) + fogBegin;
    float normZ = saturate((linearDepth - fogBegin) / (fogEnd - fogBegin));

// 2) uv 구성
    float3 uv = float3(input.texcoord, normZ);
    float4 scatteringColorAndTransmittance = fogVolume.Sample(linearClampSampler, uv);
    float3 scatteringColor = HDR(scatteringColorAndTransmittance.rgb);

    return float4(scatteringColor, scatteringColorAndTransmittance.a);
}