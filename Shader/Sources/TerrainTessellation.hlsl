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
    float3 normal : NORMAL;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    uint instanceID : SV_INSTANCEID;
};

struct Terrain_HIN
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
    uint instanceID : INSTANCEID;
};

struct Terrain_DIN
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
    uint instanceID : INSTANCEID;
};

struct Terrain_PIN
{
    float4 position : SV_POSITION;
    float3 wPosition : POSITION1;
    float3 vPosition : POSITION2;
    float3 normal : NORMAL;
    float2 texcoord1 : TEXCOORD0;
    float2 texcoord2 : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
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

Terrain_HIN Terrain_VS(Terrain_VIN input)
{
    Terrain_HIN output;
    
    output.position = float4(input.position, 1.f);
    output.normal = input.normal;
    output.texcoord1 = input.texcoord1;
    output.texcoord2 = input.texcoord2;
    output.tangent = input.tangent;
    output.bitangent = input.bitangent;
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

    
    //tess.EdgeTess[0] = GetTessFactor(0.5f * mul((patch[0].position + patch[4].position), world));
    //tess.EdgeTess[1] = GetTessFactor(0.5f * mul((patch[0].position + patch[20].position), world));
    //tess.EdgeTess[2] = GetTessFactor(0.5f * mul((patch[4].position + patch[24].position), world));
    //tess.EdgeTess[3] = GetTessFactor(0.5f * mul((patch[20].position + patch[24].position), world));
    
    
    //float3 sum = float3(0.0f, 0.0f, 0.0f);
    //for (int i = 0; i < 25; i++)
    //    sum += mul(patch[i].position, world).xyz;
    //float3 center = sum / 25.0f;
    //tess.InsideTess[0] = tess.InsideTess[1] = GetTessFactor(float4(center, 1.0f));
    
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
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(25)]
[patchconstantfunc("Constant_HS")]
[maxtessfactor(64.f)]
Terrain_DIN Terrain_HS(InputPatch<Terrain_HIN, 25> patch, uint pointID : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    Terrain_DIN output;
    
    output.position = patch[pointID].position;
    output.normal = patch[pointID].normal;
    output.texcoord1 = patch[pointID].texcoord1;
    output.texcoord2 = patch[pointID].texcoord2;
    output.tangent = patch[pointID].tangent;
    output.bitangent = patch[pointID].bitangent;
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

void CubicBernstein(float t, out float B[4], out float dB[4])
{
    float it = 1 - t;
    B[0] = it * it * it;
    B[1] = 3 * t * it * it;
    B[2] = 3 * t * t * it;
    B[3] = t * t * t;
    
    dB[0] = -3 * it * it;
    dB[1] = 3 * it * it - 6 * t * it;
    dB[2] = 6 * t * it - 3 * t * t;
    dB[3] = 3 * t * t;
}

float3 dPatchPositionU(OutputPatch<Terrain_DIN, 25> patch, float Bu[4], float Bv[4], float dBu[4])
{
    float3 sum = 0;
    [unroll]
    for (int j = 0; j < 4; ++j)
    [unroll]
        for (int i = 0; i < 4; ++i)
            sum += dBu[i] * Bv[j] * patch[i * 5 + j].position;
    return sum;
}

float3 dPatchPositionV( OutputPatch<Terrain_DIN, 25> patch, float Bu[4], float Bv[4], float dBv[4])
{
    float3 sum = 0;
    [unroll]
    for (int j = 0; j < 4; ++j)
    [unroll]
        for (int i = 0; i < 4; ++i)
            sum += Bu[i] * dBv[j] * patch[i * 5 + j].position;
    return sum;
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

float3 CubicBezierSumNormal(OutputPatch<Terrain_DIN, 25> patch, float basisU[5], float basisV[5])
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    
    sum = basisV[0] * (basisU[0] * patch[0].normal + basisU[1] * patch[1].normal + basisU[2] * patch[2].normal + basisU[3] * patch[3].normal + basisU[4] * patch[4].normal);
    sum += basisV[1] * (basisU[0] * patch[5].normal + basisU[1] * patch[6].normal + basisU[2] * patch[7].normal + basisU[3] * patch[8].normal + basisU[4] * patch[9].normal);
    sum += basisV[2] * (basisU[0] * patch[10].normal + basisU[1] * patch[11].normal + basisU[2] * patch[12].normal + basisU[3] * patch[13].normal + basisU[4] * patch[14].normal);
    sum += basisV[3] * (basisU[0] * patch[15].normal + basisU[1] * patch[16].normal + basisU[2] * patch[17].normal + basisU[3] * patch[18].normal + basisU[4] * patch[19].normal);
    sum += basisV[4] * (basisU[0] * patch[20].normal + basisU[1] * patch[21].normal + basisU[2] * patch[22].normal + basisU[3] * patch[23].normal + basisU[4] * patch[24].normal);
    
    return sum;
}

[domain("quad")]
Terrain_PIN Terrain_DS(PatchTessFactor tess, float2 uv : SV_DomainLocation, const OutputPatch<Terrain_DIN, 25> patch)
{
    Terrain_PIN o;


    float basisU[5], basisV[5];
    BernsteinBasis(uv.x, basisU);
    BernsteinBasis(uv.y, basisV);

    float3 worldPos = CubicBezierSum(patch, basisU, basisV);
    o.position = mul(float4(worldPos, 1), modelContexts[patch[0].instanceID].world);
    o.wPosition = o.position.xyz;
    o.vPosition = mul(o.position, view).xyz;
    o.position = mul(o.position, viewProjection);
    o.material = modelContexts[patch[0].instanceID].material;


    float Bu4[4], Bv4[4], dBu4[4], dBv4[4];
    CubicBernstein(uv.x, Bu4, dBu4);
    CubicBernstein(uv.y, Bv4, dBv4);

    float3 dpdu = dPatchPositionU(patch, Bu4, Bv4, dBu4);
    float3 dpdv = dPatchPositionV(patch, Bu4, Bv4, dBv4);


    float3x3 W = (float3x3) modelContexts[patch[0].instanceID].world;
    float3 Nw = normalize(mul(W, CubicBezierSumNormal(patch, basisU, basisV)));
    float3 Tu = mul(W, dpdu);
    float3 Tn = normalize(Tu - Nw * dot(Nw, Tu));
    float3 Bn = normalize(cross(Nw, Tn));

    o.normal = Nw;
    o.tangent = Tn;
    o.bitangent = Bn;


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


Deffered_POUT Terrain_PS(Terrain_PIN input)
{
    Deffered_POUT output = (Deffered_POUT) 0;

    [unroll]
    for (int i = 0; i < isShadow; ++i)
    {
        float depth = input.position.z;
        output.diffuse = float4(depth, depth, depth, 1.0f);
        return output;
    }

   
    const MaterialConstants matConst = materialConstants[input.material];

    float4 baseColor = textures[matConst.diffuseTexture[0]].Sample(linearWrapSampler, input.texcoord1);
    float4 detailColor = textures[matConst.diffuseTexture[1]].Sample(anisotropicWrapSampler, input.texcoord2);
    float4 normalSample = textures[matConst.normalTexture[0]].Sample(anisotropicWrapSampler, input.texcoord2);

    float3 normalTS = normalSample.xyz * 2.0f - 1.0f; 
    
    normalTS.xy *= 5.f; //  X,Y 성분 확대
    normalTS = normalize(normalTS);
    

    float3 T = normalize(input.tangent);
    float3 B = normalize(input.bitangent);
    float3 N = normalize(input.normal);
    float3x3 TBN = float3x3(T, B, N);

    float3 finalNormal = normalize(mul(normalTS, TBN));

    output.diffuse = saturate(baseColor * 0.3f + detailColor * 0.7f);
    output.normal = float4(finalNormal, 0.0f);
    output.position = float4(input.wPosition, 1.0f);
    output.emissive = float4(0, 0, 0, 0);

    return output;
}