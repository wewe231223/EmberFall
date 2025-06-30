cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    matrix middleViewProj;
    matrix prevViewProj;

    float3 cameraPosition;
    int isShadow;
    float3 shadowOffset;

}

struct ModelContext
{
    matrix world;
    float3 BBCenter; 
    float3 BBExtents;
    uint material;
};

struct MaterialConstants
{
    float4 diffuse;
    float4 specular;
    float4 emissive;
    
    uint diffuseTexture[8];
    uint specularTexture[8];
    uint metalicTexture[8];
    uint emissiveTexture[8];
    uint normalTexture[8];
    uint alphaTexture[8];
};

struct SkyBox_VIN
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    uint instanceID : SV_INSTANCEID;
};

struct SkyBox_VOUT
{
    float4 position : SV_Position;
    float4 curPosition : POSITION0;
    float4 prevPosition : POSITION1;
    float2 texcoord : TEXCOORD;
    uint material : MATERIALID;
};

struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
    float4 emissive : SV_TARGET3;
    float4 velocity : SV_TARGET4;
};


StructuredBuffer<ModelContext> modelContexts : register(t0);
StructuredBuffer<MaterialConstants> materialConstants : register(t1);
Texture2D textures[1024] : register(t2, space0);
StructuredBuffer<ModelContext> prevModelContexts : register(t2, space1);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);


SkyBox_VOUT SkyBox_VS(SkyBox_VIN input)
{
    ModelContext modelContext = modelContexts[0];

    SkyBox_VOUT output;
    output.position = mul(float4(input.position, 1.f), modelContext.world);
    output.prevPosition = mul(mul(float4(input.position, 1.f), prevModelContexts[0].world), prevViewProj);

    output.position = mul(output.position, viewProjection);
    output.curPosition = output.position;
    
    output.texcoord = input.texcoord;
    output.material = modelContext.material;
    
    
    
    return output;
}

Deffered_POUT SkyBox_PS(SkyBox_VOUT input)
{
    
    Deffered_POUT output = (Deffered_POUT)0;
    
   
    output.diffuse = textures[materialConstants[input.material].diffuseTexture[0]].Sample(linearClampSampler, input.texcoord);
    
    output.normal = float4(0.0f, 0.0f, 0.0f, 5.0f);
    
    float4 curNDC = input.curPosition / input.curPosition.w;
    float4 prevNDC = input.prevPosition / input.prevPosition.w;
    
    curNDC.xy = curNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    prevNDC.xy = prevNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    float2 velocity = (curNDC.xy - prevNDC.xy) * 50.0f;
    output.velocity = float4(velocity, 0.0f, 1.0f);
    return output;
}