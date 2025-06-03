#define CP_COUNT_PER_ROW 129
#define PATCH_LENGTH 4

cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    matrix middleViewProjection;

    float3 cameraPosition;
    int isShadow;
}

struct ModelContext
{
    matrix world;
    float3 BBCenter;
    float3 BBExtents;
    uint material;
    uint xSegIndex;
    uint zSegIndex; 
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

struct Terrain_VIN
{
    float3 position : POSITION;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    uint instanceID : SV_INSTANCEID;
};

struct Terrain_HIN
{
    float4 position : POSITION;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    uint instanceID : INSTANCEID;
};

struct Terrain_DIN
{
    float4 position : POSITION;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    uint instanceID : INSTANCEID;
};

struct Terrain_PIN
{
    float4 position : SV_POSITION;
    float3 wPosition : POSITION1;
    float3 vPosition : POSITION2;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    uint material : MATERIALID;
};

struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
    float4 emissive : SV_TARGET3;
};

StructuredBuffer<float3> globalPositions : register(t0);
StructuredBuffer<ModelContext> modelContexts : register(t1);
StructuredBuffer<MaterialConstants> materialConstants : register(t2);
Texture2D textures[1024] : register(t3);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

Terrain_HIN Terrain_VS(Terrain_VIN input)
{
    Terrain_HIN output;
    
    output.position = float4(input.position, 1.f);
    output.texcoord1 = input.texcoord1;
    output.texcoord2 = input.texcoord2;
    output.instanceID = input.instanceID;
    
    return output;
}

struct PatchTessFactor
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

float GetTessFactor(float4 center)
{
    return 16.f;
    
    float fDistToCamera = distance(center.xyz, cameraPosition);
    float s = saturate((fDistToCamera - 10.0f) / (500.0f - 10.0f));

    return (lerp(64.0f, 1.0f, s));
}

PatchTessFactor Constant_HS(InputPatch<Terrain_HIN, 25> patch, uint patchID : SV_PrimitiveID)
{
    PatchTessFactor tess;
    
    matrix world = modelContexts[patch[0].instanceID].world;
    
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    [unroll] 
    for (int i = 0; i < 25; i++)
    {
        sum += mul(patch[i].position, modelContexts[patch[i].instanceID].world).xyz;
    }
    float3 center = sum / 25.0f;
    
    tess.EdgeTess[0] = GetTessFactor(float4(center, 1.0f));
    tess.EdgeTess[1] = tess.EdgeTess[2] = tess.EdgeTess[3] = tess.EdgeTess[0];
    tess.InsideTess[0] = tess.InsideTess[1] = tess.EdgeTess[0];
    
    return tess;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(25)]
[patchconstantfunc("Constant_HS")]
[maxtessfactor(64.f)]
Terrain_DIN Terrain_HS(InputPatch<Terrain_HIN, 25> patch, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    Terrain_DIN output;
    
    output.position = patch[pointID].position;
    output.texcoord1 = patch[pointID].texcoord1;
    output.texcoord2 = patch[pointID].texcoord2;
    output.instanceID = patch[pointID].instanceID;
    
    return output;
}

void BernsteinBasis(float t, out float basis[5])
{
    float invT = 1.f - t;
    
    basis[0] = invT * invT * invT * invT;
    basis[1] = 4.f * t * invT * invT * invT;
    basis[2] = 6.f * t * t * invT * invT;
    basis[3] = 4.f * t * t * t * invT;
    basis[4] = t * t * t * t;
}

float3 CubicBezierSum(OutputPatch<Terrain_DIN, 25> patch, float basisU[5], float basisV[5])
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    
    sum = basisV[0] * (basisU[0] * patch[0].position + basisU[1] * patch[1].position + basisU[2] * patch[2].position + basisU[3] * patch[3].position + basisU[4] * patch[4].position);
    sum += basisV[1] * (basisU[0] * patch[5].position + basisU[1] * patch[6].position + basisU[2] * patch[7].position + basisU[3] * patch[8].position + basisU[4] * patch[9].position);
    sum += basisV[2] * (basisU[0] * patch[10].position + basisU[1] * patch[11].position + basisU[2] * patch[12].position + basisU[3] * patch[13].position + basisU[4] * patch[14].position);
    sum += basisV[3] * (basisU[0] * patch[15].position + basisU[1] * patch[16].position + basisU[2] * patch[17].position + basisU[3] * patch[18].position + basisU[4] * patch[19].position);
    sum += basisV[4] * (basisU[0] * patch[20].position + basisU[1] * patch[21].position + basisU[2] * patch[22].position + basisU[3] * patch[23].position + basisU[4] * patch[24].position);
    
    return sum;
}

float2 ComputeHeightmapUV(float3 worldPos)
{
    float u = (worldPos.x + 512.5) / 1025.0;
    float v = (worldPos.z + 512.5) / 1025.0;
    return float2(u, v);
}

[domain("quad")]
Terrain_PIN Terrain_DS(
    PatchTessFactor tess,
    float2 uv : SV_DomainLocation,
    const OutputPatch<Terrain_DIN, 25> patch)
{
    Terrain_PIN o;
    const ModelContext ctx = modelContexts[patch[0].instanceID];

    float basisU5[5], basisV5[5];
    
    BernsteinBasis(uv.x, basisU5);
    BernsteinBasis(uv.y, basisV5);
    
    float3 worldPos = CubicBezierSum(patch, basisU5, basisV5);
    float2 puv = ComputeHeightmapUV(worldPos);
    
    float4 worldPos4 = mul(float4(worldPos, 1), ctx.world);
    o.position = mul(worldPos4, viewProjection);
    o.wPosition = worldPos4.xyz;
    o.vPosition = mul(worldPos4, view).xyz;
    o.material = ctx.material;

    o.texcoord1 = lerp(
        lerp(patch[0].texcoord1, patch[4].texcoord1, uv.x),
        lerp(patch[20].texcoord1, patch[24].texcoord1, uv.x),
        uv.y);
    o.texcoord2 = lerp(
        lerp(patch[0].texcoord2, patch[4].texcoord2, uv.x),
        lerp(patch[20].texcoord2, patch[24].texcoord2, uv.x),
        uv.y);
    
    return o;
}

#define ENABLE_SPLATTING 1

Deffered_POUT Terrain_PS(Terrain_PIN input)
{
    Deffered_POUT output = (Deffered_POUT) 0;

    [unroll]
    for (int i = 0; i < isShadow; ++i)
    {
        float d = input.position.z;
        output.diffuse = float4(d, d, d, 1);
        return output;
    }

    const MaterialConstants mat = materialConstants[input.material];

#if ENABLE_SPLATTING
    float4 splat = textures[mat.alphaTexture[0]].Sample(linearWrapSampler, input.texcoord1);
    splat = saturate(splat);
    splat /= max(splat.r + splat.g + splat.b + splat.a, 1e-5);
#else
    float4 splat = float4(1, 0, 0, 0);
#endif

    float4 baseColor = 0;
    float3 blendedTS = 0;
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float4 lc = textures[mat.diffuseTexture[i]].Sample(anisotropicWrapSampler, input.texcoord2);
        float3 ln = textures[mat.normalTexture[i]].Sample(anisotropicWrapSampler, input.texcoord2).xyz * 2 - 1;
        ln.xy *= 20;
        ln = normalize(ln);

        float w = (i == 0) ? splat.r : (i == 1) ? splat.g : splat.b;
        baseColor += float4(lc.rgb, 1) * w;
        blendedTS += ln * w;
    }
    blendedTS = normalize(blendedTS);

    float3 N = normalize(textures[mat.metalicTexture[0]].Sample(linearWrapSampler, input.texcoord1).xyz); 
    float3 T = normalize(textures[mat.metalicTexture[1]].Sample(linearWrapSampler, input.texcoord1).xyz);
    float3 B = normalize(textures[mat.metalicTexture[2]].Sample(linearWrapSampler, input.texcoord1).xyz);
    float3x3 TBN = float3x3(T, B, N);

    float3 finalNormalWS = normalize(mul(blendedTS, TBN));

    output.diffuse = baseColor;
    output.normal = float4(finalNormalWS, 0);
    output.position = float4(input.wPosition, 1);
    output.emissive = float4(0, 0, 0, 0);

    return output;
}
