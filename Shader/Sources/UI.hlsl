struct ModelContext2D
{
    float3x3 transform;
    float3x3 uvTransform; 
    uint imageIndex; 
    float greyScale;
};

StructuredBuffer<ModelContext2D> modelContexts2D : register(t0);
Texture2D textures[1024] : register(t1);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

struct UI_VIN
{
    float2 position : POSITION;
    uint instanceID : SV_InstanceID;
};

struct UI_VOUT
{
    float4 position : SV_POSITION;
    uint imageIndex : IMAGEINDEX;
    float greyScale : GREYSCALE;
    float2 tex : TEXCOORD;
};


UI_VOUT UI_VS(UI_VIN input) {
    UI_VOUT Out;

    float3 pos = mul(float3(input.position, 1.f), modelContexts2D[input.instanceID].transform);
    Out.position = float4(pos.xy, 1.f, 1.f);
    Out.imageIndex = modelContexts2D[input.instanceID].imageIndex;
    Out.tex = mul(float3(input.position, 1.f), modelContexts2D[input.instanceID].uvTransform).xy;
    Out.greyScale = modelContexts2D[input.instanceID].greyScale;
    
    return Out;
}

float4 UI_PS(UI_VOUT input) : SV_TARGET 
{
    float4 color = textures[input.imageIndex].Sample(pointWrapSampler, input.tex);
    return float4(color.rgb * input.greyScale, color.a); 
}