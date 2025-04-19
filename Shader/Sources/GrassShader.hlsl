cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    float3 cameraPosition;
    int isShadow;
}

cbuffer TerrainGlobal : register(b1)
{
    int globalWidth;
    int globalHeight;
    float gridSpacing;
    float minX;
    float minZ;
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
    float2 position; // x z 좌표 
    float scale; 
    uint tex; 
};


StructuredBuffer<float3> TerrainVertices : register(t0);
StructuredBuffer<GrassPosition> grassVertices : register(t1);
StructuredBuffer<MaterialConstants> materialConstants : register(t2);
Texture2D textures[1024] : register(t3);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);


float GetHeight(float x, float z)
{
    // 로컬 좌표 계산
    float localX = x - minX;
    float localZ = z - minZ;

    // 그리드상의 부동소수점 좌표
    float fcol = localX / gridSpacing;
    float frow = localZ / gridSpacing;

    // 정수 인덱스는 소수점 부분을 버림(floor) 처리
    int col = (int) fcol;
    int row = (int) frow;

    // 인덱스를 범위 내로 제한 (분기 없는 clamp 함수 사용)
    col = clamp(col, 0, globalWidth - 2);
    row = clamp(row, 0, globalHeight - 2);

    // 보간 계수 계산
    float t = fcol - col;
    float u = frow - row;

    // 인덱스 계산
    int index00 = row * globalWidth + col;
    int index10 = index00 + 1;
    int index01 = index00 + globalWidth;
    int index11 = index01 + 1;

    // x 방향 선형 보간
    float y0 = lerp(TerrainVertices[index00].y, TerrainVertices[index10].y, t);
    float y1 = lerp(TerrainVertices[index01].y, TerrainVertices[index11].y, t);

    // z 방향 보간하여 최종 높이 산출
    return lerp(y0, y1, u);
}

struct Payload
{
    uint baseIndex;
    bool culled; 
};

#define GRASS_GRID_COUNT 2500
#define GRASS_PER_GRID 2048
#define GRASS_PER_DISPATCH 64
#define GRASS_CULL_DISTANCE 100.0f
#define GRASS_COUNT GRASS_PER_GRID * GRASS_GRID_COUNT
#define MAX_VERTEX_COUNT (GRASS_PER_DISPATCH * 4)
#define MAX_INDEX_COUNT (GRASS_PER_DISPATCH * 2)


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
    uint texIndex : TEXID;
};

[outputtopology("triangle")]
[numthreads(GRASS_PER_DISPATCH, 1, 1)]
void mainMS(
    uint3 threadId : SV_DispatchThreadID,
    uint3 groupThreadId : SV_GroupThreadID,
    in payload Payload pl,
    out indices uint3 outIndices[MAX_INDEX_COUNT],
    out vertices VSOutput outVerts[MAX_VERTEX_COUNT])
{
    const uint localId = groupThreadId.x;
    const uint grassIndex = pl.baseIndex + localId;

    if (grassIndex >= GRASS_COUNT)
        return;

    SetMeshOutputCounts(pl.culled ? 0 : MAX_VERTEX_COUNT, pl.culled ? 0 : MAX_INDEX_COUNT);

    if (pl.culled) 
        return; 
    
    GrassPosition grass = grassVertices[grassIndex];
    float3 basePos = float3(grass.position.x, GetHeight(grass.position.x, grass.position.y), grass.position.y);

    float halfSize = grass.scale * 2.f * 0.5f;
    
    basePos += halfSize; 
    float3 up = float3(0, 1, 0);
    float3 toCamera = normalize(cameraPosition - basePos);
    float3 right = normalize(cross(up, toCamera));

    float3 v0 = basePos + (-right + up) * halfSize;
    float3 v1 = basePos + (right + up) * halfSize;
    float3 v2 = basePos + (right - up) * halfSize;
    float3 v3 = basePos + (-right - up) * halfSize;

    uint vtxBase = localId * 4;
    uint idxBase = localId * 2;


    outVerts[vtxBase + 0].position = mul(float4(v0, 1.0), viewProjection);
    outVerts[vtxBase + 1].position = mul(float4(v1, 1.0), viewProjection);
    outVerts[vtxBase + 2].position = mul(float4(v2, 1.0), viewProjection);
    outVerts[vtxBase + 3].position = mul(float4(v3, 1.0), viewProjection);

    outVerts[vtxBase + 0].uv = float2(0, 0);
    outVerts[vtxBase + 1].uv = float2(1, 0);
    outVerts[vtxBase + 2].uv = float2(1, 1);
    outVerts[vtxBase + 3].uv = float2(0, 1);

    for (int i = 0; i < 4; ++i)
        outVerts[vtxBase + i].texIndex = grass.tex;

    outIndices[idxBase + 0] = uint3(vtxBase + 2, vtxBase + 1, vtxBase + 0);
    outIndices[idxBase + 1] = uint3(vtxBase + 0, vtxBase + 3, vtxBase + 2);
}

// Pixel Shader
float4 mainPS(VSOutput input) : SV_Target
{
    Texture2D tex = textures[materialConstants[materialIndex].diffuseTexture[input.texIndex]];
    float4 color = tex.Sample(anisotropicWrapSampler, input.uv);
    clip(color.a - 0.2f);
    return color;
}