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
    uint material; 
};

struct MaterialConstants
{
    float4 diffuse;
    float4 specular;
    float4 emissive;
    
    uint diffuseTexture[3];
    uint specularTexture[3];
    uint metalicTexture[3];
    uint emissiveTexture[3];
    uint normalTexture[3];
    uint alphaTexture[3];
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

float4 Standard_PS(Standard_VOUT input) : SV_TARGET {
    float4 color = textures[materialConstants[input.material].diffuseTexture[0]].Sample(linearWrapSampler, input.texcoord);
    // color += materialConstants[input.material].diffuse;
    return color;
}