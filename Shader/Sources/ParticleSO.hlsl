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
    float velocity : VELOCITY;
    
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
    float drag : DRAG;
    float opacity : OPACITY;
};

struct ParticleSO_GS_IN
{
    float3 position : POSITION;
    float halfWidth : WIDTH;
    
    float3 direction : DIRECTION;
    float velocity : VELOCITY;
    
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
    float drag : DRAG;
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

void Gravity(inout ParticleVertex v)
{
    float3 g = float3(0, -9.8f, 0);
    float3 vel = v.direction * v.velocity + g * deltaTime;
    v.direction = normalize(vel);
    v.velocity = length(vel);
    v.position += vel * deltaTime;
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
    
    
    // 이 위치로 이동
    if (emitter.lifetime <= 0.f)
    {
        ParticleVertex p = (ParticleVertex) 0;

        // 초기 방향: 위쪽 중심으로 약간의 흔들림
        float3 up = float3(0, 1.0f, 0);
        float3 jitter = float3(
            GenerateRandomInRange(-0.3f, 0.3f, vertexID * 13),
            GenerateRandomInRange(0.1f, 0.2f, vertexID * 19), // Y 흔들림도 추가
            GenerateRandomInRange(-0.3f, 0.3f, vertexID * 17)
        );
        p.direction = normalize(up + jitter);
        

        p.velocity = GenerateRandomInRange(10.f, 14.f, vertexID);

        p.position = emitter.position;
        p.halfWidth = emitter.halfWidth;
        p.halfHeight = emitter.halfHeight;
        p.material = emitter.material;
        p.spritable = emitter.spritable;
        p.spriteFrameInRow = emitter.spriteFrameInRow;
        p.spriteFrameInCol = emitter.spriteFrameInCol;
        p.spriteDuration = emitter.spriteDuration;

        p.totalLifetime = GenerateRandomInRange(3.f, 6.f, vertexID);
        p.lifetime = p.totalLifetime;

        p.type = ParticleType_ember;
        p.emitType = ParticleType_ember;
        p.remainEmit = 0;
        p.emitIndex = emitter.emitIndex;

        emitter.lifetime = 0.5f;
        if (emitter.remainEmit > 0)
            emitter.remainEmit--;

        stream.Append(p);
    }

    emitter.lifetime -= deltaTime; // ★ 이걸 넣어야 에미터가 시간이 흐르면서 다시 쏘게 됨
    stream.Append(emitter); // ★ 최종적으로 감소된 emitter 를 Append
}

//----------------------------------------------------------[ Ember Particle Update ]----------------------------------------------------------

void EmberParticleUpdate(inout ParticleVertex v, inout PointStream<ParticleVertex> stream)
{
    if (v.lifetime >= 0.f)
    {
        ParticleVertex n = v;
        Gravity(n);
        OnTerrain(n);
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

    if (outP.type == ParticleType_emit)
    {
        EmitParticleUpdate(outP, input[0].vertexID, output);
    }
    else if (outP.type == ParticleType_ember)
    {
        EmberParticleUpdate(outP, output);
    }
}