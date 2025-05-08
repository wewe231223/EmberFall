cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    Matrix middleViewProjection;
    //Matrix farViewProjection;

    float3 cameraPosition;
    int isShadow;

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

struct StandardNormal_VIN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    uint instanceID : SV_INSTANCEID;
};

struct StandardNormal_VOUT
{
    float4 position : SV_POSITION;
    float3 wPosition : POSITION1;
    float3 vPosition : POSITION2;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    uint material : MATERIALID;
};

struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
    float4 emissive : SV_TARGET3;
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

StandardNormal_VOUT StandardNormal_VS(StandardNormal_VIN input) {
    ModelContext modelContext = modelContexts[input.instanceID];

    StandardNormal_VOUT output;
    output.position = mul(float4(input.position, 1.f), modelContext.world);
    output.wPosition = output.position.xyz; 
    output.vPosition = mul(output.position, view).xyz; 
    //output.position = mul(output.position, projection);
    output.position = mul(output.position, viewProjection);
    
    
    output.normal = mul(input.normal, (float3x3)modelContext.world);
    output.tangent = mul(input.tangent, (float3x3) modelContext.world);
    output.bitangent = mul(input.bitangent, (float3x3) modelContext.world);
    output.texcoord = input.texcoord;
    output.material = modelContext.material;
    
    return output;
}


float4 Fog(float4 Color, float Distance, float fogStart, float fogEnd)
{
    float fogFactor = saturate((fogEnd - Distance) / (fogEnd - fogStart));
    return lerp(Color, float4(0.5, 0.5, 0.5, 1.0), 1 - fogFactor);
}

Deffered_POUT StandardNormal_PS(StandardNormal_VOUT input) {
    Deffered_POUT output = (Deffered_POUT) 0;
    
    float4 color = textures[materialConstants[input.material].diffuseTexture[0]].Sample(anisotropicWrapSampler, input.texcoord);
    // color += materialConstants[input.material].diffuse;
    
    clip(color.a - 0.2f);

    [unroll]
    for (int i = 0; i < isShadow; ++i)
    {
        float depth = input.position.z;
        output.diffuse = float4(depth, depth, depth, 1.0f);
        return output;
    }


    output.diffuse = color; 
    float3 normal = textures[materialConstants[input.material].normalTexture[0]].Sample(anisotropicWrapSampler, input.texcoord).rgb;
    normal = 2.0f * normal - 1.0f;
    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
    output.normal = float4(mul(normal, TBN), 1.0f);
    output.position = float4(input.wPosition, 1.0f);
    float4 emissiveColor = materialConstants[input.material].emissive;
    
    float isEmissive = step(1.0f, emissiveColor.a);
    [unroll]
    for (int i = 0; i < isEmissive; ++i)
    {
        output.emissive = float4(emissiveColor.rgb, 1.0f);
    }
    
    return output;
}