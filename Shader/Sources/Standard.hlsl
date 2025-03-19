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

struct Standard_VIN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    uint instanceID : SV_INSTANCEID;
};

struct Standard_VOUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    uint material : MATERIALID;
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

Standard_VOUT Standard_VS(Standard_VIN input) {
    ModelContext modelContext = modelContexts[input.instanceID];

    Standard_VOUT output;
    output.position = mul(float4(input.position, 1.f), modelContext.world);
    //output.position = mul(output.position, view);
    //output.position = mul(output.position, projection);
    output.position = mul(output.position, viewProjection);
    
    output.normal = mul(input.normal, (float3x3)modelContext.world);
    output.texcoord = input.texcoord;
    output.material = modelContext.material;
    
    return output;
}

Deffered_POUT Standard_PS(Standard_VOUT input) {
    Deffered_POUT output = (Deffered_POUT) 0;
    
    float4 color = textures[materialConstants[input.material].diffuseTexture[0]].Sample(anisotropicWrapSampler, input.texcoord);
    // color += materialConstants[input.material].diffuse;
    
    if (color.a < 0.1f)
    {
        discard;
    }
    
    output.diffuse = color;
    return output;
}