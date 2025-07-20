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

cbuffer Time : register(b1)
{
    uint globalTime; // milliseconds 
};

cbuffer MaterialIndex : register(b2)
{
    uint materialIndex;
};

cbuffer GrassMeta : register(b3)
{
    uint totalGrassCount;
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

StructuredBuffer<float3> grassVertices : register(t0);
StructuredBuffer<MaterialConstants> materialConstants : register(t1);
Texture2D textures[1024] : register(t2);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

#define GRASS_PER_DISPATCH 16
#define MAX_VERTEX_COUNT (GRASS_PER_DISPATCH * 8)
#define MAX_INDEX_COUNT  (GRASS_PER_DISPATCH * 12)

struct VSOutput
{
    float4 position : SV_Position;
    float4 curPosition : POSITION0;
    float4 prevPosition : POSITION1;
    float3 vPosition : POSITION2;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 wPosition : POSITION3;
    uint texIndex : TEXID;
};

float hash(uint x)
{
    x ^= x >> 17;
    x *= 0xed5ad4bb;
    x ^= x >> 11;
    x *= 0xac4c1b51;
    x ^= x >> 15;
    x *= 0x31848bab;
    x ^= x >> 14;
    return frac(x * (1.0 / 4294967296.0));
}

uint GetIndexFromFloat3(float3 v)
{
    float hash = frac(dot(v, float3(12.9898, 78.233, 37.719)));
    return (uint) (hash * 4.0);
}

#define MAX_SCALE_DIST 20.f

#define WIND_STRENGTH 0.2f   
#define WIND_FREQ     0.002f 
[outputtopology("triangle")]
[numthreads(GRASS_PER_DISPATCH, 1, 1)]
void mainMS(
    uint3 threadId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    out indices uint3 outIndices[MAX_INDEX_COUNT],
    out vertices VSOutput outVerts[MAX_VERTEX_COUNT])
{
    const uint localId = groupThreadId.x;
    const uint grassIndex = groupId.x * GRASS_PER_DISPATCH + localId;

    bool isValid = grassIndex < totalGrassCount;

    // 반드시 조건문 밖에서 호출해야 함
    SetMeshOutputCounts(isValid ? 8 : 0, isValid ? 12 : 0);

    if (!isValid)
    {
        return;
    }

    float3 grass = grassVertices[grassIndex];

    float randSize = hash(grassIndex + 12345); // seed offset
    float maxSize = lerp(0.4f, 0.6f, randSize);

    float3 toCam = cameraPosition - grass;
    float dist = length(toCam);
    
    float scaleFactor = min(1.0f, MAX_SCALE_DIST / dist);
    float halfSize = 0.5f * maxSize * scaleFactor;

    
    float randPhase = hash(grassIndex);
    float sway = sin(globalTime * WIND_FREQ + randPhase * 6.2831f) * (WIND_STRENGTH * halfSize);
    float3 swayDir = float3(0.0f, 0.0f, 1.0f);
    float3 swayOffset = swayDir * sway;

    float3 basePos = grass;
    basePos.y += halfSize;

    const float3 up = float3(0.0f, 1.0f, 0.0f);

    float randRotation = hash(grassIndex) * 6.2831f;
    float cosTheta = cos(randRotation);
    float sinTheta = sin(randRotation);

    float3 right = float3(cosTheta, 0.0f, -sinTheta);
    float3 forward = float3(sinTheta, 0.0f, cosTheta);

    float3 verts[8];

    verts[0] = basePos + (-right + up) * halfSize + swayOffset;
    verts[1] = basePos + (right + up) * halfSize + swayOffset;
    verts[2] = basePos + (right - up) * halfSize;
    verts[3] = basePos + (-right - up) * halfSize;

    verts[4] = basePos + (-forward + up) * halfSize + swayOffset;
    verts[5] = basePos + (forward + up) * halfSize + swayOffset;
    verts[6] = basePos + (forward - up) * halfSize;
    verts[7] = basePos + (-forward - up) * halfSize;

    float3 edge1_r = verts[1] - verts[0];
    float3 edge2_r = verts[2] - verts[0];
    float3 normal1 = normalize(cross(edge1_r, edge2_r));

    float3 edge1_f = verts[5] - verts[4];
    float3 edge2_f = verts[6] - verts[4];
    float3 normal2 = normalize(cross(edge1_f, edge2_f));

    const uint vtxBase = localId * 8;
    const uint idxBase = localId * 4;

    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        outVerts[vtxBase + i].position = mul(float4(verts[i], 1.0f), viewProjection);
        outVerts[vtxBase + i].wPosition = verts[i];
        outVerts[vtxBase + i].texIndex = GetIndexFromFloat3(grass);
        outVerts[vtxBase + i].curPosition = mul(float4(verts[i], 1.0f), viewProjection);
        outVerts[vtxBase + i].vPosition = mul(float4(verts[i], 1.0f), view).xyz;
        outVerts[vtxBase + i].prevPosition = mul(float4(verts[i], 1.0f), prevViewProjection);

        if (i % 4 == 0)
            outVerts[vtxBase + i].uv = float2(0, 0);
        if (i % 4 == 1)
            outVerts[vtxBase + i].uv = float2(1, 0);
        if (i % 4 == 2)
            outVerts[vtxBase + i].uv = float2(1, 1);
        if (i % 4 == 3)
            outVerts[vtxBase + i].uv = float2(0, 1);

        outVerts[vtxBase + i].normal = (i < 4) ? normal1 : normal2;
    }

    outIndices[idxBase + 0] = uint3(vtxBase + 2, vtxBase + 1, vtxBase + 0);
    outIndices[idxBase + 1] = uint3(vtxBase + 0, vtxBase + 3, vtxBase + 2);
    outIndices[idxBase + 2] = uint3(vtxBase + 6, vtxBase + 5, vtxBase + 4);
    outIndices[idxBase + 3] = uint3(vtxBase + 4, vtxBase + 7, vtxBase + 6);
}

struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
    float4 emissive : SV_TARGET3;
    float4 velocity : SV_TARGET4;
};

Deffered_POUT mainPS(VSOutput input)
{
    Texture2D tex = textures[materialConstants[materialIndex].diffuseTexture[input.texIndex]];
    float4 color = tex.Sample(anisotropicWrapSampler, input.uv);

    clip(color.a - 0.5f);

    Deffered_POUT output = (Deffered_POUT) 0;
    output.diffuse = color;
    output.normal = float4(input.normal, 1.f);
    output.position = float4(input.wPosition, 1.f);

    float4 curNDC = input.curPosition / input.curPosition.w;
    float4 prevNDC = input.prevPosition / input.prevPosition.w;
    
    curNDC.xy = curNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    prevNDC.xy = prevNDC.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    
    float4 velocity = (curNDC - prevNDC);
    
    
    output.velocity = float4(velocity.x, velocity.y, input.vPosition.z, input.position.z);
    
    return output;
}
