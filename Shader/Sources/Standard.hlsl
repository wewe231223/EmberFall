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
struct ModelContext
{
    float4x4 prevWorld;
    float4x4 world;
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

struct Standard_VIN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    uint instanceID : SV_INSTANCEID;
};

struct Standard_VOUT
{
    float4 position : SV_Position;
    float3 wPosition : POSITION0;
    float3 vPosition : POSITION1;
    float4 curPosition : POSITION2;
    float4 prevPosition : POSITION3;
    float3 normal : NORMAL;
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

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

Standard_VOUT Standard_VS(Standard_VIN input)
{
    ModelContext modelContext = modelContexts[input.instanceID];

    Standard_VOUT output;
    output.position = mul(float4(input.position, 1.f), modelContext.world);

    output.prevPosition = mul(mul(float4(input.position, 1.0f), modelContext.prevWorld), prevViewProjection);
    //output.prevPosition = mul(output.position, prevViewProj);
    output.wPosition = output.position.xyz; 
    output.vPosition = mul(output.position, view).xyz; 
    //output.position = mul(output.position, projection);
    output.position = mul(output.position, viewProjection);
    output.curPosition = output.position;
    
    output.normal = mul(input.normal, (float3x3) modelContext.world);
    output.texcoord = input.texcoord;
    output.material = modelContext.material;
    
    return output;
}

float4 Fog(float4 Color, float Distance, float fogStart, float fogEnd)
{
    float fogFactor = saturate((fogEnd - Distance) / (fogEnd - fogStart));
    return lerp(Color, float4(0.5, 0.5, 0.5, 1.0), 1 - fogFactor);
}

Deffered_POUT Standard_PS(Standard_VOUT input)
{
    Deffered_POUT output = (Deffered_POUT) 0;
    
    float4 color = textures[materialConstants[input.material].diffuseTexture[0]].Sample(anisotropicWrapSampler, input.texcoord);
    
    clip(color.a - 0.2f);

    [unroll]
    for (int i = 0; i < isShadow; ++i)
    {
        float depth = input.position.z;
        output.diffuse = float4(depth, depth, depth, 1.0f);
        return output;
    }

    output.diffuse = color;
    output.normal = float4(input.normal, 1.0f);
    output.position = float4(input.wPosition, 1.0f);

    float4 emissiveColor = materialConstants[input.material].emissive;
    
    float isEmissive = step(1.0f, emissiveColor.a);
    [unroll]
    for (int i = 0; i < isEmissive; ++i)
    {
        output.emissive = textures[materialConstants[input.material].emissiveTexture[0]].Sample(linearWrapSampler, input.texcoord) * 20.f;
    }

    
    float4 curNDC = input.curPosition / input.curPosition.w;
    float4 prevNDC = input.prevPosition / input.prevPosition.w;
    
    curNDC.xy = curNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    prevNDC.xy = prevNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    float4 velocity = (curNDC - prevNDC);
   
    output.velocity = float4(velocity.x, velocity.y, input.vPosition.z, input.position.z);

    
    return output;
}
