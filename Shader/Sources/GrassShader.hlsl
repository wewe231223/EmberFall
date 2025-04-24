cbuffer CameraCB : register(b0)
{
    matrix view;
    matrix proj;
    matrix viewProj;
    matrix middleViewProjection;
    float3 cameraPosition;
    int isShadow;
};

cbuffer Time
{
    uint globalTime; // milliseconds 
};

cbuffer MaterialIndex : register(b2)
{
    uint materialIndex; 
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

struct GrassPosition
{
    float3 position;
    float scale; 
    uint tex; 
};


StructuredBuffer<GrassPosition> grassVertices : register(t0);
StructuredBuffer<MaterialConstants> materialConstants : register(t1);
Texture2D textures[1024] : register(t2);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);


struct Payload
{
    uint baseIndex;
    bool culled; 
};

#define GRASS_GRID_COUNT 2500
#define GRASS_PER_DISPATCH 16

// 이 상수를 통해 그리드 당 풀의 개수를 제어함. 
// 이 상수는 반드시 GRASS_PER_DISPATCH 의 배수여야 함 ( GRASS_PER_DISPATCH <= 16 ) 
#define GRASS_PER_GRID 400
#define GRASS_CULL_DISTANCE 200.0f
#define GRASS_COUNT GRASS_PER_GRID * GRASS_GRID_COUNT
#define MAX_VERTEX_COUNT (GRASS_PER_DISPATCH * 8)
#define MAX_INDEX_COUNT  (GRASS_PER_DISPATCH * 12)


[numthreads(1, 1, 1)]
void mainAS(uint3 groupId : SV_GroupID)
{
    uint gridIndex = groupId.x;

    float2 gridMin = float2(-250.0f, -250.0f);
    float gridSize = 10.0f;

    uint gridX = gridIndex % 50;
    uint gridZ = gridIndex / 50;

    float2 centerXZ = gridMin + float2((gridX + 0.5f) * gridSize, (gridZ + 0.5f) * gridSize);


    float2 toCamera = centerXZ - cameraPosition.xz;
    float distSq = dot(toCamera, toCamera);

    Payload pl;
    if (distSq < GRASS_CULL_DISTANCE * GRASS_CULL_DISTANCE)
    {
        pl.baseIndex = gridIndex * GRASS_PER_GRID;
        pl.culled = false; 
    }
    else
    {
        pl.baseIndex = 0;
        pl.culled = true; 
    }
    

    DispatchMesh(GRASS_PER_GRID / GRASS_PER_DISPATCH, 1, 1, pl); 
}



struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
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

[outputtopology("triangle")]
[numthreads(GRASS_PER_DISPATCH, 1, 1)]
void mainMS(
    uint3 threadId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 groupId : SV_GroupID,
    in payload Payload pl,
    out indices uint3 outIndices[MAX_INDEX_COUNT],
    out vertices VSOutput outVerts[MAX_VERTEX_COUNT])
{
    const uint localId = groupThreadId.x;
    const uint groupIndex = threadId.x / GRASS_PER_DISPATCH;
    const uint offsetInGrid = localId;
    // baseIndex + (Mesh Shader Group Id * GRASS_PER_DISPATCH) + localId
    const uint grassIndex = pl.baseIndex + groupIndex * GRASS_PER_DISPATCH + offsetInGrid;

   
    bool shouldCull = pl.culled;
    SetMeshOutputCounts(shouldCull ? 0 : 8, shouldCull ? 0 : 12);

    if (shouldCull)
        return;
                                                                                                    
    GrassPosition grass = grassVertices[grassIndex];
    
    float rand = hash(grassIndex);
    
    float3 basePos = grass.position;
    float halfSize = grass.scale * 0.6f;
   // rand < 0.5 ? halfSize = 0.f : halfSize = halfSize; 
    basePos.y += halfSize;

    // 고정 방향 기반으로 mesh 잔디 구성
    const float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = float3(1.0f, 0.0f, 0.0f); // X 방향
    float3 forward = float3(0.0f, 0.0f, 1.0f); // Z 방향
    
  
    // 2장 교차하는 평면 잔디 (8 정점)
    float3 verts[8];
    verts[0] = basePos + (-right + up) * halfSize;
    verts[1] = basePos + (right + up) * halfSize;
    verts[2] = basePos + (right - up) * halfSize;
    verts[3] = basePos + (-right - up) * halfSize;

    verts[4] = basePos + (-forward + up) * halfSize;
    verts[5] = basePos + (forward + up) * halfSize;
    verts[6] = basePos + (forward - up) * halfSize;
    verts[7] = basePos + (-forward - up) * halfSize;

    // 교차 평면 normal 계산
    float3 normal1 = normalize(cross(right, up)); // right 평면
    float3 normal2 = normalize(cross(forward, up)); // forward 평면
    
    uint vtxBase = localId * 8;
    uint idxBase = localId * 4;

    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        outVerts[vtxBase + i].position = mul(float4(verts[i], 1.0f), viewProj);
        outVerts[vtxBase + i].texIndex = grass.tex;
        
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

    // Plane 1 (right 방향)
    outIndices[idxBase + 0] = uint3(vtxBase + 2, vtxBase + 1, vtxBase + 0);
    outIndices[idxBase + 1] = uint3(vtxBase + 0, vtxBase + 3, vtxBase + 2);

    // Plane 2 (forward 방향)
    outIndices[idxBase + 2] = uint3(vtxBase + 6, vtxBase + 5, vtxBase + 4);
    outIndices[idxBase + 3] = uint3(vtxBase + 4, vtxBase + 7, vtxBase + 6);
}



struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
};

// Pixel Shader
Deffered_POUT mainPS(VSOutput input)
{
    Texture2D tex = textures[materialConstants[materialIndex].diffuseTexture[input.texIndex]];
    float4 color = tex.Sample(anisotropicWrapSampler, input.uv);
    
    clip(color.a - 0.5f);
    
    Deffered_POUT output = (Deffered_POUT) 0;
    output.diffuse = color;
    output.normal = float4(input.normal, 1.f);
    output.position = input.position;
    
    return output; 
}