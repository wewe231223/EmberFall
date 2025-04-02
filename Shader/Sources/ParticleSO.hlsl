#define ParticleType_emit 1
#define ParticleType_shell 2
#define ParticleType_ember 3

#define ember_LifeTime 2.f

#define RANDOM_BUFFER_SIZE 4096
cbuffer GlobalCB : register(b0)
{
    float globalTime; // 초 단위 
    float deltaTime; // 초 단위 
}


StructuredBuffer<float> RandomBuffer : register(t0);


struct EmitParticleContext
{
    float3 position;
    uint flag; 
};

StructuredBuffer<EmitParticleContext> EmitPosition : register(t1);

cbuffer TerrainGlobal : register(b1)
{
    int globalWidth;
    int globalHeight;
    float gridSpacing;
    float minX;
    float minZ;
};

StructuredBuffer<float3> TerrainVertices : register(t2);

struct ParticleVertex
{
    float3 position : POSITION;
    float halfWidth : WIDTH;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;
    
    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;
    
    float3 direction : DIRECTION;
    float velocity : VELOCITY;
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;
    
    uint type : PARTICLETYPE;
    uint emitType : EMITTYPE;
    uint remainEmit : REMAINEMIT;
    uint emitIndex : EMITINDEX;
};

struct ParticleSO_GS_IN
{
    float3 position : POSITION;
    float halfWidth : WIDTH;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;
    
    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;
    
    float3 direction : DIRECTION;
    float velocity : VELOCITY;
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;
    
    uint type : PARTICLETYPE;
    uint emitType : EMITTYPE;
    uint remainEmit : REMAINEMIT;
    uint emitIndex : EMITINDEX;
    
    uint vertexID : VERTEXID;
};


//----------------------------------------------------------[ Random ]----------------------------------------------------------
// 해시 함수
uint FastHash(uint seed)
{
    uint hash = seed * 1665u + 101423u; // 선형 합동 생성기
    return hash ^ (hash >> 16); // 비트 셔플
}

// 랜덤 값 추출 함수
float FetchRandomValue(uint index)
{
    return RandomBuffer[index % 4096]; // 4096으로 나눠 순환
}

// 난수 생성
float GenerateRandom(uint seed)
{
    // 시간을 밀리초 단위로 변환하고 정수화
    uint timeSeed = uint(globalTime * 1000.f);
 //   uint deltaSeed = uint(deltaTime * 1000.f);

    // 시드 결합 후 해시 생성
    uint combinedSeed = FastHash(seed) ^ FastHash(timeSeed);
    return FetchRandomValue(combinedSeed); // [0, 1] 범위의 난수 반환
}

// 특정 범위 내 난수 생성
float GenerateRandomInRange(float min, float max, uint seed)
{
    float randomValue = GenerateRandom(seed);
    return lerp(min, max, randomValue); // [min, max] 범위로 매핑
}

// 랜덤 방향 생성
float3 GenerateRandomDirection(uint seed)
{
    // 두 개의 난수 값 생성
    float rand1 = GenerateRandom(seed); // 첫 번째 난수
    float rand2 = GenerateRandom(seed + 1); // 두 번째 난수

    // [-1, 1] 범위로 변환
    rand1 = rand1 * 2.0 - 1.0;
    rand2 = rand2 * 2.0 - 1.0;

    // 랜덤 방향 계산
    float z = rand1; // -1에서 1까지의 Z 축 값
    float xyMagnitude = sqrt(max(1.0 - z * z, 0.0)); // XY 평면상의 반지름

    // 랜덤 각도 (theta)
    float theta = rand2 * 3.14159; // [0, π] 범위

    float3 direction = float3(
        xyMagnitude * cos(theta), // X 값
        xyMagnitude * sin(theta), // Y 값
        z // Z 값
    );

    return normalize(direction); // 단위 벡터로 정규화
}
//----------------------------------------------------------[ Random ]----------------------------------------------------------

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

void Gravity(inout ParticleVertex vertex)
{
    float3 gravity = float3(0.f, -9.8f, 0.f);
    float3 velocity = vertex.direction * vertex.velocity;
    velocity += gravity * deltaTime;
    vertex.direction = normalize(velocity);
    vertex.velocity = length(velocity);
    vertex.position += velocity * deltaTime;
}


void FoggyEmberParticleUpdate(inout ParticleVertex vertex, inout PointStream<ParticleVertex> stream, uint vertexID)
{
    if (vertex.lifetime >= 0.f)
    {
        ParticleVertex n = vertex;

        // 아주 약한 부력처럼 천천히 위로
        float riseSpeed = 0.5f; // 천천히 상승 (초당 0.5m)
        float3 velocity = n.direction * n.velocity;
        velocity.y = riseSpeed;

        // 약간의 랜덤 흔들림 (바람 효과)
        float randomX = GenerateRandomInRange(-0.1f, 0.1f, vertexID * 13);
        float randomZ = GenerateRandomInRange(-0.1f, 0.1f, vertexID * 17);
        velocity.x += randomX;
        velocity.z += randomZ;

        n.position += velocity * deltaTime;

        // (옵션) lifetime에 따라 점점 velocity 줄이기
        float lifeRatio = saturate(n.lifetime / n.totalLifetime);
        n.velocity *= lifeRatio;

        stream.Append(n);
    }
}



ParticleSO_GS_IN ParticleSOPassVS(ParticleVertex input, uint vertedID : SV_VertexID)
{
    ParticleSO_GS_IN output = (ParticleSO_GS_IN) 0;
    
    output.position = input.position;
    output.halfWidth = input.halfWidth;
    output.halfHeight = input.halfHeight;
    output.material = input.material;
    output.spritable = input.spritable;
    output.spriteFrameInRow = input.spriteFrameInRow;
    output.spriteFrameInCol = input.spriteFrameInCol;
    output.spriteDuration = input.totalLifetime;
    output.direction = input.direction;
    output.velocity = input.velocity;
    output.totalLifetime = input.totalLifetime;
    output.lifetime = input.lifetime;
    output.type = input.type;
    output.emitType = input.emitType;
    output.remainEmit = input.remainEmit;
    output.emitIndex = input.emitIndex;
    output.vertexID = vertedID;
    
    return output;
}

void AppendVertex(inout ParticleVertex vertex, inout PointStream<ParticleVertex> stream)
{
    stream.Append(vertex);
}

void EmitParticleUpdate(inout ParticleVertex vertex, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    vertex.position = EmitPosition[vertex.emitIndex].position; 
    
    ParticleVertex newParticle = (ParticleVertex) 0;
    if (vertex.lifetime <= 0.f)
    {
        newParticle.position = vertex.position;
        newParticle.halfWidth = vertex.halfWidth;
        newParticle.halfHeight = vertex.halfHeight;
        newParticle.material = vertex.material;
        newParticle.spritable = vertex.spritable;
        newParticle.spriteFrameInRow = vertex.spriteFrameInRow;
        newParticle.spriteFrameInCol = vertex.spriteFrameInCol;
        newParticle.spriteDuration = vertex.spriteDuration;
        
        newParticle.direction = GenerateRandomDirection(vertexID);
        newParticle.direction.y = 1.f; 
        newParticle.direction = normalize(newParticle.direction);
        
        newParticle.velocity = GenerateRandomInRange(15.f, 20.f, vertexID);
        newParticle.totalLifetime = GenerateRandomInRange(5.f, 7.f, vertexID);
        newParticle.lifetime = newParticle.totalLifetime;
        newParticle.type = ParticleType_ember;
        newParticle.emitType = ParticleType_ember;
        newParticle.remainEmit = 0;
        
        vertex.lifetime = vertex.totalLifetime;
        vertex.remainEmit--;

        
        AppendVertex(newParticle, stream);
    }
   
    AppendVertex(vertex, stream);
    
}

void EmberParticleUpdate(inout ParticleVertex vertex, inout PointStream<ParticleVertex> stream)
{
    if (vertex.lifetime >= 0.f)
    {
        ParticleVertex n = vertex; 
        
        Gravity(n);
        
        float h = GetHeight(n.position.x, n.position.z);
        
        if (n.position.y < h + n.halfHeight)
        {
            n.position.y = h + n.halfHeight;
            n.velocity = 0.f;
        }
        
        stream.Append(n);
    }
}


[maxvertexcount(16)]
void ParticleSOPassGS(point ParticleSO_GS_IN input[1], inout PointStream<ParticleVertex> output)
{
    ParticleVertex outPoint = (ParticleVertex) 0;
    
    outPoint.position = input[0].position;
    
    outPoint.halfWidth = input[0].halfWidth;
    outPoint.halfHeight = input[0].halfHeight;
    outPoint.material = input[0].material;
    outPoint.spritable = input[0].spritable;
    outPoint.spriteFrameInRow = input[0].spriteFrameInRow;
    outPoint.spriteFrameInCol = input[0].spriteFrameInCol;
    outPoint.spriteDuration = input[0].spriteDuration;
    outPoint.direction = input[0].direction;
    outPoint.velocity = input[0].velocity;
    outPoint.totalLifetime = input[0].totalLifetime;
    outPoint.lifetime = input[0].lifetime - deltaTime;
    outPoint.type = input[0].type;
    outPoint.emitType = input[0].emitType;
    outPoint.remainEmit = input[0].remainEmit;
    outPoint.emitIndex = input[0].emitIndex;
   
    
    
    if (outPoint.type == ParticleType_emit)
        EmitParticleUpdate(outPoint, input[0].vertexID, output);
    else if (outPoint.type == ParticleType_ember)
        FoggyEmberParticleUpdate(outPoint, output, input[0].vertexID);
}
