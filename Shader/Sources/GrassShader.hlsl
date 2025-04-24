cbuffer CameraCB : register(b0)
{
    matrix view;
    matrix proj;
    matrix viewProj;
    matrix middleViewProjection;
    float3 cameraPosition;
    int isShadow;
};


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
    float3 position;
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


struct Payload
{
    uint baseIndex;
    bool culled; 
};

#define GRASS_GRID_COUNT 2500
#define GRASS_PER_GRID 512
#define GRASS_PER_DISPATCH 64
#define GRASS_CULL_DISTANCE 200.0f
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

    SetMeshOutputCounts(pl.culled ? 0 : MAX_VERTEX_COUNT, pl.culled ? 0 : MAX_INDEX_COUNT);

    if (pl.culled) 
        return; 
    
    GrassPosition grass = grassVertices[grassIndex];
    float3 basePos = grass.position;

    float halfSize = grass.scale * 0.5f;
    
    basePos.y += halfSize; 
    float3 toCameraXZ = normalize(float3(cameraPosition.x - basePos.x, 0.0f, cameraPosition.z - basePos.z));
    float3 right = float3(toCameraXZ.z, 0.0f, -toCameraXZ.x);
    const float3 up = float3(0.0f, 1.0f, 0.0f);

    float3 v0 = basePos + (-right + up) * halfSize;
    float3 v1 = basePos + (right + up) * halfSize;
    float3 v2 = basePos + (right - up) * halfSize;
    float3 v3 = basePos + (-right - up) * halfSize;

    uint vtxBase = localId * 4;
    uint idxBase = localId * 2;


    outVerts[vtxBase + 0].position = mul(float4(v0, 1.0), viewProj);
    outVerts[vtxBase + 1].position = mul(float4(v1, 1.0), viewProj);
    outVerts[vtxBase + 2].position = mul(float4(v2, 1.0), viewProj);
    outVerts[vtxBase + 3].position = mul(float4(v3, 1.0), viewProj);

    outVerts[vtxBase + 0].uv = float2(0, 0);
    outVerts[vtxBase + 1].uv = float2(1, 0);
    outVerts[vtxBase + 2].uv = float2(1, 1);
    outVerts[vtxBase + 3].uv = float2(0, 1);

    [unroll]
    for (int i = 0; i < 4; ++i)
        outVerts[vtxBase + i].texIndex = grass.tex;

    outIndices[idxBase + 0] = uint3(vtxBase + 2, vtxBase + 1, vtxBase + 0);
    outIndices[idxBase + 1] = uint3(vtxBase + 0, vtxBase + 3, vtxBase + 2);
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
    output.normal = float4(0.0f, 1.0f, 0.0f, 5.0f);
    
    return output; 
}