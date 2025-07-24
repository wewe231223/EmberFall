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
    float2 texcoord2 : TEXCOORD1;
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
    float2 texcoord2 : TEXCOORD1;
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

Standard_VOUT ArenaGround_VS(Standard_VIN input)
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
    output.texcoord2 = input.texcoord2;
    output.material = modelContext.material;
    
    return output;
}

float4 Fog(float4 Color, float Distance, float fogStart, float fogEnd)
{
    float fogFactor = saturate((fogEnd - Distance) / (fogEnd - fogStart));
    return lerp(Color, float4(0.5, 0.5, 0.5, 1.0), 1 - fogFactor);
}

Deffered_POUT ArenaGround_PS(Standard_VOUT input)
{
    Deffered_POUT output = (Deffered_POUT) 0;

    [unroll]
    for (int i = 0; i < isShadow; ++i)
    {
        float depth = input.position.z;
        output.diffuse = float4(depth, depth, depth, 1.0f);
        return output;
    }
    
    const MaterialConstants mat = materialConstants[input.material];


    float4 splat = textures[mat.alphaTexture[0]].Sample(linearWrapSampler, input.texcoord);
    splat = saturate(splat);
    splat /= max(splat.r + splat.g + splat.b + splat.a, 1e-5);

    float4 baseColor = 0;
    float3 blendedTS = 0;
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float4 lc = textures[mat.diffuseTexture[i]].Sample(anisotropicWrapSampler, input.texcoord2);
        float3 ln = textures[mat.normalTexture[i]].Sample(anisotropicWrapSampler, input.texcoord2).xyz * 2 - 1;
        ln.xy *= 30;
        ln = normalize(ln);

        float w = (i == 0) ? splat.r : (i == 1) ? splat.g : splat.b;
        baseColor += float4(lc.rgb, 1) * w;
        blendedTS += ln * w;
    }
    blendedTS = normalize(blendedTS);

    float3 N = float3(0.f, 1.f, 0.f);
    float3 T = float3(1.f, 0.f, 0.f);
    float3 B = float3(0.f, 0.f, -1.f);
    float3x3 TBN = float3x3(T, B, N);

    float3 finalNormalWS = normalize(mul(blendedTS, TBN));
    

    output.diffuse = baseColor;
    output.normal = float4(finalNormalWS, 1.0f);
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
   
    output.velocity = float4(velocity.x, velocity.y, 0.0f, input.position.z);

    
    return output;
}
