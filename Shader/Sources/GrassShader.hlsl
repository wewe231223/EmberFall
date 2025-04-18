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
    uint grassIndex;
};

#define GRASS_COUNT 1000000
#define GRASS_CULL_DISTANCE 100.0f


// Amplification Shader
[numthreads(1, 1, 1)]
void mainAS(uint3 id : SV_DispatchThreadID)
{
    Payload sPayload; 
    sPayload.grassIndex = 0xFFFFFFFF;
    
    float2 pos = grassVertices[id.x].position;
    float2 deltaXZ = pos - cameraPosition.xz;
    float distSq = dot(deltaXZ, deltaXZ);

    if (distSq < GRASS_CULL_DISTANCE * GRASS_CULL_DISTANCE)
    {
        sPayload.grassIndex = id.x;
    }
    
    DispatchMesh(1, 1, 1, sPayload);
}


// Mesh Shader
struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    uint texIndex : TEXID;
};


[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void mainMS(
            uint3 id : SV_DispatchThreadID, 
            in payload Payload pl,
            out indices uint3 outIndices[2],
            out vertices VSOutput outVerts[4])
{
    if(pl.grassIndex == 0xFFFFFFFF)
        return;
    
    GrassPosition grass = grassVertices[pl.grassIndex];
    float3 basePos = float3(grass.position.x, GetHeight(grass.position.x, grass.position.y), grass.position.y);

    float halfSize = grass.scale * 0.5f;
    float3 up = float3(0, 1, 0);

    // 카메라 방향 기반 billboard right 벡터 계산
    float3 toCamera = normalize(cameraPosition - basePos);
    float3 right = normalize(cross(up, toCamera));

    float3 v0 = basePos + (-right + up) * halfSize;
    float3 v1 = basePos + (right + up) * halfSize;
    float3 v2 = basePos + (right - up) * halfSize;
    float3 v3 = basePos + (-right - up) * halfSize;
    
    SetMeshOutputCounts(4, 2);

    outVerts[0].position = float4(v0, 1.0);
    outVerts[1].position = float4(v1, 1.0);
    outVerts[2].position = float4(v2, 1.0);
    outVerts[3].position = float4(v3, 1.0);

    outVerts[0].uv = float2(0, 0);
    outVerts[1].uv = float2(1, 0);
    outVerts[2].uv = float2(1, 1);
    outVerts[3].uv = float2(0, 1);

    outVerts[0].texIndex = grass.tex;
    outVerts[1].texIndex = grass.tex;
    outVerts[2].texIndex = grass.tex;
    outVerts[3].texIndex = grass.tex;

    outIndices[0] = uint3(0, 1, 2);
    outIndices[1] = uint3(2, 3, 0);
}

// Pixel Shader
float4 mainPS(VSOutput input) : SV_Target
{
    Texture2D tex = textures[input.texIndex];
    float4 color = tex.Sample(anisotropicWrapSampler, input.uv);
    clip(color.a - 0.2f);
    return color;
}