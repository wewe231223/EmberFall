cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    float3 cameraPosition;
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
    uint texIndex   : TEXINDEX;
    float2 texcoord : TEXCOORD;
    uint instanceID : SV_INSTANCEID;
};

struct SkyBox_VOUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    uint material : MATERIALID;
    uint imageID : IMAGEID;
};

struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
};


StructuredBuffer<ModelContext> modelContexts : register(t0);
StructuredBuffer<MaterialConstants> materialConstants : register(t1);
Texture2D textures[1024] : register(t2);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

SkyBox_VOUT SkyBox_VS(SkyBox_VIN input)
{
    // SkyBox is Unique Entity
    ModelContext modelContext = modelContexts[0];

    SkyBox_VOUT output;
    output.position = mul(float4(input.position, 1.f), modelContext.world);
    output.position = mul(output.position, viewProjection);
    
    output.texcoord = input.texcoord;
    output.material = modelContext.material;
    output.imageID = materialConstants[modelContext.material].diffuseTexture[input.texIndex];
    
    return output;
}

Deffered_POUT SkyBox_PS(SkyBox_VOUT input)
{
    Deffered_POUT output = (Deffered_POUT)0;
    output.diffuse = textures[input.imageID].Sample(pointWrapSampler, input.texcoord);
    return output;
}