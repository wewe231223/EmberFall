#define ParticleType_emit   1
#define ParticleType_shell  2
#define ParticleType_ember  3

#define ember_LifeTime      2.f

#define RANDOM_BUFFER_SIZE  4096
#define NULL_INDEX 0xFFFFFFFF

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
    
    float3 direction : DIRECTION;
    float3 velocity : VELOCITY;
    
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;

    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;

    uint type : PARTICLETYPE;
    uint emitType : EMITTYPE;
    uint remainEmit : REMAINEMIT;
    uint emitIndex : EMITINDEX;

    float mass : MASS;
    float3 drag : DRAG;
    float opacity : OPACITY;
};

struct ParticleSO_GS_IN
{
    float3 position : POSITION;
    float halfWidth : WIDTH;
    
    float3 direction : DIRECTION;
    float3 velocity : VELOCITY;
    
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;

    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;

    uint type : PARTICLETYPE;
    uint emitType : EMITTYPE;
    uint remainEmit : REMAINEMIT;
    uint emitIndex : EMITINDEX;

    float mass : MASS;
    float3 drag : DRAG;
    float opacity : OPACITY;
    
    uint vertexID : VERTEXID;
};

//----------------------------------------------------------[ Random ]----------------------------------------------------------

uint FastHash(uint seed)
{
    uint hash = seed * 1665u + 101423u;
    return hash ^ (hash >> 16);
}

float FetchRandomValue(uint index)
{
    return RandomBuffer[index % RANDOM_BUFFER_SIZE];
}

float GenerateRandom(uint seed)
{
    uint timeSeed = uint(globalTime * 1000.f);
    uint combinedSeed = FastHash(seed) ^ FastHash(timeSeed);
    return FetchRandomValue(combinedSeed);
}

float GenerateRandomInRange(float min, float max, uint seed)
{
    float r = GenerateRandom(seed);
    return lerp(min, max, r);
}

float3 GenerateRandomDirection(uint seed)
{
    float rand1 = GenerateRandom(seed) * 2.0 - 1.0;
    float rand2 = GenerateRandom(seed + 1) * 2.0 - 1.0;
    float z = rand1;
    float xy = sqrt(max(1.0 - z * z, 0.0));
    float theta = rand2 * 3.14159;
    float3 dir = float3(xy * cos(theta), xy * sin(theta), z);
    return normalize(dir);
}

//----------------------------------------------------------[ Terrain Height ]----------------------------------------------------------

float GetHeight(float x, float z)
{
    float localX = x - minX;
    float localZ = z - minZ;
    float fcol = localX / gridSpacing;
    float frow = localZ / gridSpacing;
    int col = clamp((int) fcol, 0, globalWidth - 2);
    int row = clamp((int) frow, 0, globalHeight - 2);
    float t = fcol - col;
    float u = frow - row;
    int idx00 = row * globalWidth + col;
    int idx10 = idx00 + 1;
    int idx01 = idx00 + globalWidth;
    int idx11 = idx01 + 1;
    float y0 = lerp(TerrainVertices[idx00].y, TerrainVertices[idx10].y, t);
    float y1 = lerp(TerrainVertices[idx01].y, TerrainVertices[idx11].y, t);
    return lerp(y0, y1, u);
}

//----------------------------------------------------------[ Physics Helpers ]----------------------------------------------------------

void ApplyPhysics(inout ParticleVertex v)
{
    float3 g = float3(0, -9.8f, 0);
    float3 dragForce = -v.drag * v.velocity;

    float3 acceleration = (g + dragForce) / max(v.mass, 0.0001f);
    v.velocity += acceleration * deltaTime;

    v.position += v.velocity * deltaTime;

    v.direction = normalize(v.velocity); // optional
}


void OnTerrain(inout ParticleVertex v)
{
    float h = GetHeight(v.position.x, v.position.z);
    if (v.position.y < h + v.halfHeight)
    {
        v.position.y = h + v.halfHeight;
        v.velocity = 0;
    }
}

//----------------------------------------------------------[ Emit Particle Update ]----------------------------------------------------------

void AppendVertex(inout ParticleVertex v, inout PointStream<ParticleVertex> stream)
{
    stream.Append(v);
}

void EmitParticleUpdate(inout ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    emitter.position = EmitPosition[emitter.emitIndex].position;

    if (emitter.lifetime <= 0.f)
    {
        ParticleVertex p = (ParticleVertex) 0;

        // 생성 위치: 지면에서 약간 위로 띄움
        float minHeight = 1.0f;
        float maxHeight = 4.0f;
        p.position = emitter.position + float3(0, GenerateRandomInRange(minHeight, maxHeight, vertexID * 101), 0);

        // 방향 (시각용): 수평 중심 + 약간 위로
        float3 spreadDir = float3(
            GenerateRandomInRange(-1.f, 1.f, vertexID * 13),
            GenerateRandomInRange(0.1f, 0.4f, vertexID * 19),
            GenerateRandomInRange(-1.f, 1.f, vertexID * 17)
        );
        p.direction = normalize(spreadDir);

        // 수명 결정
        float baseLife = 5.0f;
        float growth = saturate(globalTime / 120.f);
        p.totalLifetime = GenerateRandomInRange(baseLife, baseLife + 3.0f + 60.0f * growth, vertexID);
        p.lifetime = p.totalLifetime;

        // 수명 기반 보간 계수 (0~1)
        float lifeFactor = saturate((p.totalLifetime - baseLife) / 60.f);

        // 속도 설정: 수명이 길수록 천천히, 위로는 높게
        float horizontalSpeed = lerp(5.f, 2.f, lifeFactor);
        float verticalSpeed = lerp(1.5f, 4.0f, lifeFactor); // 오래 뜨도록 높게
        p.velocity = float3(
            GenerateRandomInRange(-horizontalSpeed, horizontalSpeed, vertexID * 13),
            GenerateRandomInRange(1.5f, verticalSpeed, vertexID * 19), // Y 속도 강조
            GenerateRandomInRange(-horizontalSpeed, horizontalSpeed, vertexID * 17)
        );

        // drag 설정: 수명이 길수록 → 수평 drag 줄이고, 수직 drag도 낮춤
        float horizontalDrag = lerp(0.6f, 0.2f, lifeFactor);
        float verticalDrag = lerp(1.2f, 0.3f, lifeFactor); // Y drag 줄여서 오래 부유

        p.drag = float3(
            GenerateRandomInRange(horizontalDrag * 0.8f, horizontalDrag * 1.2f, vertexID * 71),
            GenerateRandomInRange(verticalDrag * 0.8f, verticalDrag * 1.2f, vertexID * 73),
            GenerateRandomInRange(horizontalDrag * 0.8f, horizontalDrag * 1.2f, vertexID * 79)
        );

        p.mass = 10.0f; // 고정 질량

        // 기타 속성 복사
        p.halfWidth = emitter.halfWidth;
        p.halfHeight = emitter.halfHeight;
        p.material = emitter.material;
        p.spritable = emitter.spritable;
        p.spriteFrameInRow = emitter.spriteFrameInRow;
        p.spriteFrameInCol = emitter.spriteFrameInCol;
        p.spriteDuration = emitter.spriteDuration;
        p.opacity = 1.0f;

        p.type = ParticleType_ember;
        p.emitType = ParticleType_ember;
        p.remainEmit = 0;
        p.emitIndex = emitter.emitIndex;

        // 에미터 재발사 대기시간
        emitter.lifetime = emitter.totalLifetime;
        if (emitter.remainEmit > 0)
            emitter.remainEmit--;

        stream.Append(p);
    }

    emitter.lifetime -= deltaTime;
    stream.Append(emitter);
}


//----------------------------------------------------------[ Ember Particle Update ]----------------------------------------------------------

void EmberParticleUpdate(inout ParticleVertex v, inout PointStream<ParticleVertex> stream)
{
    if (v.lifetime >= 0.f)
    {
        ParticleVertex n = v;

        ApplyPhysics(n); // 중력 + 공기저항
        OnTerrain(n); // 지면 충돌 처리

        stream.Append(n);
    }
}

//----------------------------------------------------------[ Stream-Out VS ]----------------------------------------------------------

ParticleSO_GS_IN ParticleSOPassVS(ParticleVertex inV, uint vid : SV_VertexID)
{
    ParticleSO_GS_IN o = (ParticleSO_GS_IN) 0;
    o.position = inV.position;
    o.halfWidth = inV.halfWidth;
    o.halfHeight = inV.halfHeight;
    o.material = inV.material;
    o.spritable = inV.spritable;
    o.spriteFrameInRow = inV.spriteFrameInRow;
    o.spriteFrameInCol = inV.spriteFrameInCol;
    o.spriteDuration = inV.totalLifetime;
    o.direction = inV.direction;
    o.velocity = inV.velocity;
    o.totalLifetime = inV.totalLifetime;
    o.lifetime = inV.lifetime;
    o.type = inV.type;
    o.emitType = inV.emitType;
    o.remainEmit = inV.remainEmit;
    o.emitIndex = inV.emitIndex;
    o.vertexID = vid;
    o.mass = inV.mass;
    o.drag = inV.drag;
    o.opacity = inV.opacity;
    
    return o;
}

//----------------------------------------------------------[ Stream-Out GS ]----------------------------------------------------------

[maxvertexcount(16)]
void ParticleSOPassGS(point ParticleSO_GS_IN input[1], inout PointStream<ParticleVertex> output)
{
    ParticleVertex outP = (ParticleVertex) 0;
    outP.position = input[0].position;
    outP.halfWidth = input[0].halfWidth;
    outP.halfHeight = input[0].halfHeight;
    outP.material = input[0].material;
    outP.spritable = input[0].spritable;
    outP.spriteFrameInRow = input[0].spriteFrameInRow;
    outP.spriteFrameInCol = input[0].spriteFrameInCol;
    outP.spriteDuration = input[0].spriteDuration;
    outP.direction = input[0].direction;
    outP.velocity = input[0].velocity;
    outP.totalLifetime = input[0].totalLifetime;
    outP.lifetime = input[0].lifetime - deltaTime;
    outP.type = input[0].type;
    outP.emitType = input[0].emitType;
    outP.remainEmit = input[0].remainEmit;
    outP.emitIndex = input[0].emitIndex;
    outP.mass = input[0].mass;
    outP.drag = input[0].drag;
    outP.opacity = input[0].opacity;

    if (outP.type == ParticleType_emit)
    {
        EmitParticleUpdate(outP, input[0].vertexID, output);
    }
    else if (outP.type == ParticleType_ember)
    {
        EmberParticleUpdate(outP, output);
    }
}