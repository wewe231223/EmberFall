cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
    float4x4 middleViewProjection;
    float4x4 prevViewProjection;

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
    uint boneStart;
};

struct BoneContext
{
    matrix prevBoneTransform;
    matrix boneTransform;
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

struct StandardAnimationNormal_VIN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    int4 boneID : BONEID;
    float4 boneWeight : BONEWEIGHT;
    uint instanceID : SV_INSTANCEID;
};

struct StandardAnimationNormal_PIN
{
    float4 position : SV_Position;
    float3 wPosition : POSITION0;
    float4 curPosition : POSITION1;
    float4 prevPosition : POSITION2;
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
    float4 velocity : SV_TARGET4;
};

StructuredBuffer<ModelContext> modelContexts : register(t0);
StructuredBuffer<MaterialConstants> materialConstants : register(t1);
Texture2D textures[1024] : register(t2, space0);

StructuredBuffer<float4x4> boneTransforms : register(t2, space1);
StructuredBuffer<float4x4> prevBoneTransforms : register(t3, space1);


SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

StandardAnimationNormal_PIN StandardAnimationNormal_VS(StandardAnimationNormal_VIN input)
{
    ModelContext modelContext = modelContexts[input.instanceID];

    StandardAnimationNormal_PIN output;
    
    float4x4 boneTransform = (float4x4)0;
    float4x4 prevBoneTransform = (float4x4)0;
    
 
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[0]] * input.boneWeight.x;
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[1]] * input.boneWeight.y;
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[2]] * input.boneWeight.z;
    boneTransform += boneTransforms[modelContext.boneStart + input.boneID[3]] * input.boneWeight.w;
    
    prevBoneTransform += prevBoneTransforms[modelContext.boneStart + input.boneID[0]] * input.boneWeight.x;
    prevBoneTransform += prevBoneTransforms[modelContext.boneStart + input.boneID[1]] * input.boneWeight.y;
    prevBoneTransform += prevBoneTransforms[modelContext.boneStart + input.boneID[2]] * input.boneWeight.z;
    prevBoneTransform += prevBoneTransforms[modelContext.boneStart + input.boneID[3]] * input.boneWeight.w;
    
    output.position = mul(float4(input.position, 1.0f), boneTransform);
    output.position = mul(output.position, modelContext.world);
    //output.prevPosition = mul(mul(mul(float4(input.position, 1.0f), boneTransform), modelContext.prevWorld), prevViewProj);
    output.prevPosition = mul(mul(mul(float4(input.position, 1.0f), prevBoneTransform), modelContext.prevWorld), prevViewProjection);
    output.wPosition = output.position.xyz;
    output.position = mul(output.position, viewProjection);
    output.curPosition = output.position;
    
    float3x3 boneWorldTransform = mul((float3x3) boneTransform, (float3x3) modelContext.world);
    
    output.normal = normalize(mul(input.normal, boneWorldTransform));
    output.tangent = normalize(mul(input.tangent, boneWorldTransform));
    output.bitangent = normalize(mul(input.bitangent, boneWorldTransform));

    output.texcoord = input.texcoord;
    output.material = modelContext.material;
    
    
    
    return output;
}

Deffered_POUT StandardAnimationNormal_PS(StandardAnimationNormal_PIN input)
{
    Deffered_POUT output = (Deffered_POUT) 0;
    
    [unroll]
    for (int i = 0; i < isShadow; ++i)
    {
        float depth = input.position.z;
        output.diffuse = float4(depth, depth, depth, 1.0f);
        return output;
    }
    
    output.diffuse = textures[materialConstants[input.material].diffuseTexture[0]].Sample(linearWrapSampler, input.texcoord);
    
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
        //output.emissive = float4(emissiveColor.rgb, 1.0f);
        output.emissive = textures[materialConstants[input.material].emissiveTexture[0]].Sample(linearWrapSampler, input.texcoord) * 20.0f;

    }
    
    float4 curNDC = input.curPosition / input.curPosition.w;
    float4 prevNDC = input.prevPosition / input.prevPosition.w;
    
    curNDC.xy = curNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    prevNDC.xy = prevNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    float4 velocity = (curNDC - prevNDC) * 3.0f;
    
    output.velocity = float4(velocity.x, velocity.y, 0.0f, input.position.z);

    
    
    return output;
}
