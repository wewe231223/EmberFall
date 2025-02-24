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

struct Terrain_VIN
{
    float3 position : POSITION;
    float2 texcoord1 : TEXCOORD1;
    float2 texcoord2 : TEXCOORD2;
    uint instanceID : SV_INSTANCEID;
};

struct Terrain_HIN
{
    float4 position : POSITION;
    float2 texcoord1 : TEXCOORD1;
    float2 texcoord2 : TEXCOORD2;
    uint instanceID : INSTANCEID;
};

struct Terrain_DIN
{
    float4 position : POSITION;
    float2 texcoord1 : TEXCOORD1;
    float2 texcoord2 : TEXCOORD2;
    uint instanceID : INSTANCEID;
};


struct Terrain_PIN
{
    float4 position : SV_POSITION;
    float2 texcoord1 : TEXCOORD1;
    float2 texcoord2 : TEXCOORD2;
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
    float fDistToCamera = distance(center.xyz, cameraPosition);
    float s = saturate((fDistToCamera - 10.0f) / (30.0f - 10.0f));

    return (lerp(64.0f, 1.0f, s));
}

PatchTessFactor Constant_HS(InputPatch<Terrain_HIN, 25> patch, uint patchID : SV_PrimitiveID)
{
    PatchTessFactor tess;
    
    matrix world = modelContexts[patch[0].instanceID].world;
    
    tess.EdgeTess[0] = GetTessFactor(0.5f * mul((patch[0].position + patch[4].position), world));
    tess.EdgeTess[1] = GetTessFactor(0.5f * mul((patch[0].position + patch[20].position), world));
    tess.EdgeTess[2] = GetTessFactor(0.5f * mul((patch[4].position + patch[24].position), world));
    tess.EdgeTess[3] = GetTessFactor(0.5f * mul((patch[20].position + patch[24].position), world));
    
    tess.InsideTess[0] = GetTessFactor(0.25f * mul((patch[0].position + patch[4].position + patch[20].position + patch[24].position), world));
    tess.InsideTess[1] = tess.InsideTess[0];
    
    return tess;
}

[domain("quad")]
[partitioning("fractional_even")]
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

float3 LineBezierSum(OutputPatch<Terrain_DIN, 25> patch, uint index[5], float basis[5])
{
    float3 sum = float3(0.f, 0.f, 0.f);
    for (int i = 0; i < 5; ++i)
    {
        sum += basis[i] * patch[index[i]].position.xyz;
    }
    return sum;
}


float3 CubicBezierSum(OutputPatch<Terrain_DIN, 25> patch, float basisU[5], float basisV[5])
{
    float3 sum = float3(0.f, 0.f, 0.f);
    for (int i = 0; i < 5; ++i)
    {
        uint index[5] = { i * 5, i * 5 + 1, i * 5 + 2, i * 5 + 3, i * 5 + 4 };
        sum += basisV[i] * LineBezierSum(patch, index, basisU);
    }
    return sum;
}

[domain("quad")]
Terrain_PIN Terrain_DS(PatchTessFactor patchTess, float2 uv : SV_DomainLocation, const OutputPatch<Terrain_DIN, 25> patch)
{
    Terrain_PIN output;
 
    float basisU[5];
    float basisV[5];
    
    BernsteinBasis(uv.x, basisU);
    BernsteinBasis(uv.y, basisV);
    
    matrix world = modelContexts[patch[0].instanceID].world;
    uint material = modelContexts[patch[0].instanceID].material;
    
    output.position = float4(CubicBezierSum(patch, basisU, basisV), 1.f);
    output.position = mul(output.position, world);
    output.position = mul(output.position, viewProjection);
    output.material = patch[0].instanceID;
    
    output.texcoord1 = lerp(
    lerp(patch[0].texcoord1, patch[4].texcoord1, uv.x),
    lerp(patch[20].texcoord1, patch[24].texcoord1, uv.x),
    uv.y);
    
    output.texcoord2 = lerp(
    lerp(patch[0].texcoord2, patch[4].texcoord2, uv.x),
    lerp(patch[20].texcoord2, patch[24].texcoord2, uv.x),
    uv.y);
    
    
    return output;
}


float4 Terrain_PS(Terrain_PIN input) : SV_TARGET
{
    return float4(1.f, 1.f, 1.f, 1.f);
}