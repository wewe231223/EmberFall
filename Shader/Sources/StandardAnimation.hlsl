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
    uint boneStart;
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

struct StandardAnimation_VIN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    int4 boneID : BONEID;
    float4 boneWeight : BONEWEIGHT;
    uint instanceID : SV_INSTANCEID;
};

struct StandardAnimation_PIN
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
Texture2D textures[1024] : register(t2, space0);

StructuredBuffer<float4x4> boneTransforms : register(t2, space1);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

StandardAnimation_PIN StandardAnimation_VS(StandardAnimation_VIN input) {
    ModelContext modelContext = modelContexts[input.instanceID];

    StandardAnimation_PIN output;
    
    float4x4 boneTransform = (float4x4)0;
    
 
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[0]] * input.boneWeight.x;
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[1]] * input.boneWeight.y;
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[2]] * input.boneWeight.z;
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[3]] * input.boneWeight.w;
    
    output.position = mul(float4(input.position, 1.0f), boneTransform);
        
    output.position = mul(output.position, modelContext.world);
    output.position = mul(output.position, viewProjection);
    
    output.normal = normalize(mul(input.normal, (float3x3)boneTransform));
    output.texcoord = input.texcoord;
    output.material = modelContext.material;
    
    return output;
}

Deffered_POUT StandardAnimation_PS(StandardAnimation_PIN input) {
    
    Deffered_POUT output = (Deffered_POUT)0;
    
    output.diffuse = textures[materialConstants[input.material].diffuseTexture[0]].Sample(linearWrapSampler, input.texcoord);
    // color += materialConstants[input.material].diffuse;
    
    
    return output;
}